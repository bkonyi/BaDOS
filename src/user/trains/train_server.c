#include <trains/train_server.h>
#include <trains/track_position_server.h>
#include <trains/train_controller_commander.h>
#include <trains/train_calibration_loader.h>
#include <trains/train_path_finder.h>
#include <io/io.h>
#include <terminal/terminal.h>
#include <terminal/terminal_debug_log.h>
#include <ring_buffer.h>
#include <task_priorities.h>
#include <trains/track_reservation_server.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))


static void train_conductor(void);

static void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, sensor_triggers_t* ,train_position_info_t* train_position_info); 
static bool handle_find_train(int16_t train, int16_t slot, int8_t* sensors, int8_t* initial_sensors, train_position_info_t* train_position_info);

static void handle_update_train_position_info(int16_t train, int16_t slot, train_position_info_t* train_position_info, int32_t time,uint32_t);
static int _train_position_update_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t V, uint32_t* av_out);
static int _train_position_get_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t* av);
static int _train_position_get_av_velocity_at_speed(train_position_info_t* tpi, track_node* from, track_node* to, int16_t speed, uint32_t* av_out);
static void handle_train_stop_around_sensor(train_position_info_t* tpi,int32_t train_number,int8_t sensor_num, int32_t mm_diff, bool use_path); 
static void handle_train_set_switch_direction(train_position_info_t* tpi, int16_t switch_num, int16_t direction);
static void handle_train_set_switch_and_reverse(train_position_info_t* train_position_info, int32_t train_number, int16_t switch_number, int8_t switch_direction, bool use_path);

static void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers,int16_t sensor_num) ;
static void _set_stop_around_trigger(train_position_info_t* tpi,sensor_triggers_t* triggers,int16_t sensor_num, int32_t mm_diff, bool use_path);
static void _handle_sensor_triggers(train_position_info_t* tpi, sensor_triggers_t* triggers,uint32_t train_number, int32_t sensor_group, int32_t sensor_index) ;
static void handle_set_stop_offset(train_position_info_t* train_position_info,int32_t mm_diff);
static int32_t _distance_to_send_stop_command(train_position_info_t* tpi,track_node* start_node,uint32_t destination_sensor_num, int32_t mm_diff,bool use_path) ;
static int _train_position_get_prev_first_av_velocity(train_position_info_t* tpi, track_node* node, uint32_t* av_out) ; 
static void handle_goto_destination(train_position_info_t* train_position_info, sensor_triggers_t* triggers, int16_t train_num, int8_t sensor_num);
static void handle_train_reversing(int16_t train, int8_t slot, train_position_info_t* train_position_info);
static void handle_stopped_at_destination(int16_t train_number, int8_t slot, train_position_info_t* train_position_info);
static void _handle_train_track_position_update(train_position_info_t* tpi);
static void _handle_train_reservations(train_position_info_t* tpi);

typedef enum train_server_cmd_t {
    TRAIN_SERVER_INIT                   = 1,
    TRAIN_SERVER_SENSOR_DATA            = 2,
    TRAIN_SERVER_SWITCH_CHANGED         = 3,
    TRAIN_SERVER_REGISTER_STOP_SENSOR   = 4,
    TRAIN_SERVER_FIND_INIT_POSITION     = 5,
    TRAIN_SERVER_DIRECTION_CHANGE       = 6,
    TRAIN_SERVER_REQUEST_CALIBRATION_INFO = 7,
    TRAIN_SERVER_STOP_AROUND_SENSOR     = 8,
    TRAIN_SERVER_SET_SPEED              = 9,
    TRAIN_SERVER_SET_STOP_OFFSET        = 10,
    TRAIN_SERVER_GOTO_DESTINATION       = 11,
    TRAIN_SERVER_SET_REVERSING          = 12,
    TRAIN_SERVER_STOPPED_AT_DESTINATION = 13
} train_server_cmd_t;

typedef struct train_server_msg_t {
    train_server_cmd_t command;
    uint32_t num1;
    uint32_t num2;
} train_server_msg_t;

typedef struct train_server_sensor_msg_t {
    train_server_cmd_t command;
    char sensors[SENSOR_MESSAGE_SIZE];
} train_server_sensor_msg_t;

typedef enum {
    CONDUCTOR_TRAIN_STOP = 0,
    CONDUCTOR_SET_SWITCH = 1,
    CONDUCTOR_REVERSE_TRAIN = 2,
    CONDUCTOR_STOPPED_AT_DESTINATION = 3
} conductor_command_t;

typedef struct {
    conductor_command_t command;
    int32_t delay;
    int32_t train_number;
    int32_t num1;
    int8_t byte1;
} conductor_info_t;

void train_position_info_init(train_position_info_t* tpi) {
    tpi->speed = 0;
    tpi->ticks_at_last_sensor = 0;
    tpi->last_sensor = NULL;
    tpi->next_sensor_estimated_time = 0;
    tpi->average_velocity = 0;
    tpi->last_stopping_distance = 0;
    tpi->next_sensor = NULL;
    tpi->sensor_error_next_sensor = NULL;
    tpi->switch_error_next_sensor = NULL;
    tpi->is_under_over = true; //Assume we are starting at speed 0 so this doesn't really matter
    tpi->stopping_offset = 0;
    tpi->ok_to_record_av_velocities = false;
    tpi->is_reversed = false;
    tpi->reverse_offset = 180; //18cm from front of pickup to rear of back wheels
    tpi->last_tick_time_seen =0;
    tpi->leading_end_node = NULL;
    tpi->leading_end_offset_in_node =0;
    tpi->stopping_distance = NULL;


    RING_BUFFER_INIT(tpi->conductor_tids, MAX_CONDUCTORS);
}

void train_server(void) {
    //The bigger of the 2 should be the size we use for receive
    int message_size = (TRAIN_SERVER_MSG_SIZE > TRAIN_SERVER_SENSOR_MSG_SIZE?
                        TRAIN_SERVER_MSG_SIZE:
                        TRAIN_SERVER_SENSOR_MSG_SIZE);
	int requester;
	char message[message_size]; 
    train_server_msg_t* train_server_message = ((train_server_msg_t*)message);
    int16_t train_number = -1; //The number associated wiht the train
    int16_t train_slot   = -1; //The slot that the train is registered to.
    int32_t distance_estimation;

    int32_t last_distance_update_time = 0; //The last time the expected distance for the train was updated
    int conductor_tid; //Used when destroying conductors
    int new_speed;
    bool finding_initial_position = false; //Is the train currently trying to find its initial location
    bool initial_sensor_reading_received = false; //Has the train gotten its first sensor update to be used for finding the train
    int8_t finding_initial_sensor_state[10]; //The first sensor update used when finding the 
    
    SENSOR_TRIGGER_INFO_INIT(sensor_triggers);

    bool is_stopping_at_landmark = false;
    

    train_position_info_t train_position_info;
    train_position_info_init(&train_position_info);


    QUEUE_INIT(train_position_info.reserved_node_queue);
    //CURRENTLY A STEM CELL TRAIN,
    //need to obtain train info
    Receive(&requester,message, message_size);
    Reply(requester,NULL,0);

    if(train_server_message->command != TRAIN_SERVER_INIT) {
        //Our train hasn't been initialized
        ASSERT(0);
    }

    train_number = train_server_message->num1; 
    train_slot   = train_server_message->num2;
    train_position_info.train_num = train_number;
    train_position_info.train_slot = train_slot;
    tps_add_train(train_number);

	FOREVER {
		Receive(&requester, message, message_size);

        if(train_server_message->command != TRAIN_SERVER_REQUEST_CALIBRATION_INFO) {
            Reply(requester, (char*)NULL, 0);
        }

        switch (train_server_message->command) {
            case TRAIN_SERVER_SENSOR_DATA:
                if(!finding_initial_position) {
                    //Do calculations for our train.
                    handle_sensor_data(train_number, train_slot, ((train_server_sensor_msg_t*)train_server_message)->sensors, &sensor_triggers, &train_position_info);
                    int32_t new_time = Time();

                    if(new_time - last_distance_update_time > 10) {
                        int32_t time_to_expected_time = train_position_info.next_sensor_estimated_time - new_time;
                        distance_estimation = (time_to_expected_time * ((int32_t)train_position_info.average_velocity)) / 100;
                        
                        //TODO: Improve this when we have acceleration profiles
                        if(distance_estimation>=0){
                            send_term_update_dist_msg(train_slot, distance_estimation);  
                        }
                        
                        last_distance_update_time = new_time;
                    }
                } else {
                    if(!initial_sensor_reading_received) {
                        memcpy(finding_initial_sensor_state, ((train_server_sensor_msg_t*)message)->sensors, sizeof(int8_t) * 10);
                        initial_sensor_reading_received = true;
                    } else {
                        finding_initial_position = !handle_find_train(train_number, train_slot, ((train_server_sensor_msg_t*)message)->sensors, finding_initial_sensor_state, &train_position_info);
                    }
                }

                break;
            case TRAIN_SERVER_SWITCH_CHANGED:
                //Invalidate any predictions we made
                //Kill our conductor
                //Resurrect him with a new delay

                //TODO figure out if this is necessary
                while(!IS_BUFFER_EMPTY(train_position_info.conductor_tids)) {
                    POP_FRONT(train_position_info.conductor_tids, conductor_tid);
                    Destroy(conductor_tid);
                }

                //TODO recalculate path finding
                ASSERT(0);
                break;
            case TRAIN_SERVER_REGISTER_STOP_SENSOR:
                _set_stop_on_sensor_trigger(&sensor_triggers, train_server_message->num1);
                break;
            case TRAIN_SERVER_FIND_INIT_POSITION:
                finding_initial_position = true;
                send_term_heavy_msg(false,"Finding train: %d", train_number);
                tcs_train_set_speed(train_number, 2);
                break;
            case TRAIN_SERVER_REQUEST_CALIBRATION_INFO:
                Reply(requester, (char*)&train_position_info.average_velocities, sizeof(avg_velocity_t) * 80 * MAX_AV_SENSORS_FROM * MAX_STORED_SPEEDS);
                break;
            case TRAIN_SERVER_SET_SPEED:
                new_speed = train_server_message->num1;

                if(new_speed < train_position_info.speed) {
                    train_position_info.is_under_over = false;
                } else if(new_speed > train_position_info.speed) {
                    train_position_info.is_under_over = true;
                } // Else leave the over under, we changed to the same speed

                train_position_info.last_speed = train_position_info.speed;
                train_position_info.speed  = new_speed;

                if(new_speed == 0) {
                    train_position_info.ok_to_record_av_velocities = false;
                }else {
                    train_position_info.ok_to_record_av_velocities = true;
                }

                if(new_speed == 0 && is_stopping_at_landmark) {
                    is_stopping_at_landmark = false;
                    int time_diff = Time() - train_position_info.ticks_at_last_sensor;
                    int distance = time_diff * train_position_info.average_velocity / 100;
                    send_term_heavy_msg(false, "Stopping train at %d.%dcm from %s Estimated Stopping Distance: %d", distance / 10, distance % 10, train_position_info.last_sensor->name, train_position_info.last_stopping_distance);
                }
                
                break;
            case TRAIN_SERVER_STOP_AROUND_SENSOR:
                _set_stop_around_trigger(&train_position_info,&sensor_triggers,((train_server_msg_t*)message)->num1,((train_server_msg_t*)message)->num2,false) ;
                is_stopping_at_landmark = true;
                break;
            case TRAIN_SERVER_SET_STOP_OFFSET:
                handle_set_stop_offset(&train_position_info, train_server_message->num1);
                break;
            case TRAIN_SERVER_GOTO_DESTINATION:
                handle_goto_destination(&train_position_info, &sensor_triggers, train_number, train_server_message->num1);
                break;
            case TRAIN_SERVER_STOPPED_AT_DESTINATION:
                handle_stopped_at_destination(train_number, train_slot, &train_position_info);
                break;
            case TRAIN_SERVER_SET_REVERSING:
                handle_train_reversing(train_number, train_slot, &train_position_info);
                break;
            default:
                //Invalid command
                bwprintf(COM2, "Invalid train command: %d from TID: %d\r\n", train_server_message->command, requester);
                ASSERT(0);
                break;
        }
	}
}

void train_server_specialize(tid_t tid, uint32_t train_num, int8_t slot) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_INIT;
    msg.num1 = train_num;
    msg.num2 = slot;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
}

void train_trigger_stop_on_sensor(tid_t tid, int8_t sensor_num) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_REGISTER_STOP_SENSOR;
    msg.num1 = sensor_num;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_find_initial_position(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_FIND_INIT_POSITION;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_send_sensor_update(tid_t tid, int8_t* sensors) {
    train_server_sensor_msg_t sensor_update;
    sensor_update.command = TRAIN_SERVER_SENSOR_DATA;
    memcpy(sensor_update.sensors, sensors, SENSOR_MESSAGE_SIZE);
    Send(tid, (char*)&sensor_update, sizeof(train_server_sensor_msg_t), NULL, 0);
}

void train_send_stop_around_sensor_msg(tid_t tid, int8_t sensor_num,int32_t mm_diff) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_STOP_AROUND_SENSOR;
    msg.num1 = sensor_num;
    msg.num2 = mm_diff;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
}

void train_request_calibration_info(tid_t tid, avg_velocity_t average_velocity_info[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS]) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_REQUEST_CALIBRATION_INFO;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)average_velocity_info, sizeof(avg_velocity_t) * 80 * MAX_AV_SENSORS_FROM * MAX_STORED_SPEEDS);
}

void train_server_set_speed(tid_t tid, uint16_t speed) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_SET_SPEED;
    msg.num1 = speed;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_server_send_set_stop_offset_msg(tid_t tid, int32_t mm_diff) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_SET_STOP_OFFSET;
    msg.num1 = mm_diff;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_server_goto_destination(tid_t tid, int8_t sensor_num) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_GOTO_DESTINATION;
    msg.num1 = sensor_num;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_server_set_reversing(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_SET_REVERSING;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_server_stopped_at_destination(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_STOPPED_AT_DESTINATION;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers, int16_t sensor_num) {
    sensor_triggers_set(triggers, sensor_num, TRIGGER_STOP_AT, NULL, NULL);
}

void _set_stop_around_trigger(train_position_info_t* tpi,sensor_triggers_t* triggers,int16_t sensor_num, int32_t mm_diff,bool use_path) {
    int32_t distance =0;
    int16_t sensor_to_trigger_at; 


    distance = _distance_to_send_stop_command(tpi,tpi->last_sensor,sensor_num,mm_diff, use_path);
    if(distance < 0){
        Delay(1000);
        ASSERT(0);
        return;
    }

    if(use_path){
        sensor_to_trigger_at =  get_sensor_before_distance_using_path(get_path_iterator(tpi->current_path,tpi->last_sensor),distance);
    }else {
        sensor_to_trigger_at =  get_sensor_before_distance(tpi->last_sensor,distance);
    }
   
    send_term_debug_log_msg("Setting trigger for stop at: %c%d at: %c%d Distance to stop: %d", sensor_id_to_letter(sensor_num), sensor_id_to_number(sensor_num), sensor_id_to_letter(sensor_to_trigger_at), sensor_id_to_number(sensor_to_trigger_at), distance);

    if(use_path){
        sensor_triggers_set(triggers,sensor_to_trigger_at,TRIGGER_STOP_AROUND_USING_PATH,((uint8_t*)&sensor_num),&mm_diff);
    }else {
        sensor_triggers_set(triggers,sensor_to_trigger_at,TRIGGER_STOP_AROUND,NULL,NULL);  
    }
    
}

int32_t _distance_to_send_stop_command(train_position_info_t* tpi,track_node* start_node,uint32_t destination_sensor_num, int32_t mm_diff,bool use_path) {
    int32_t distance = 0;

    track_node * destination_sensor = get_sensor_node_from_num(start_node,destination_sensor_num); 

    if(start_node == destination_sensor) {
        //TODO: MODIFY FOR USE WITH PATH
        start_node = get_next_sensor(start_node);
    }

    //We couldn't find a node...
    if(start_node == NULL) {
        ASSERT(0);
        return -1;
    }

    //Get distance to that point
    if(use_path) {
        distance = distance_between_track_nodes_using_path(get_path_iterator(tpi->current_path, start_node),destination_sensor);
    }else {
        distance = distance_between_track_nodes(start_node,destination_sensor,false);
    }
    
    track_node * runoff_limit = NULL;
    if(use_path){
        runoff_limit = get_next_sensor_or_exit_using_path(get_path_iterator(tpi->current_path,destination_sensor));
    }
   if(runoff_limit == NULL){
        runoff_limit = get_next_sensor_or_exit(destination_sensor);
        use_path = false;
    }
    

    uint32_t runoff_length =0;
    if(runoff_limit != NULL){
        if(use_path) {
            runoff_length = distance_between_track_nodes_using_path(get_path_iterator(tpi->current_path,destination_sensor),runoff_limit);
        }else {
            runoff_length = distance_between_track_nodes(destination_sensor,runoff_limit,false);
        }
    }
    if(mm_diff > runoff_length) {
        mm_diff = runoff_length;
    }

    distance += mm_diff;

    //account for the stopping_offset on this track
    distance += tpi->stopping_offset;

    if(tpi->speed != 0) {
        distance -= tpi->stopping_distance(tpi->speed, false);
    }
    
    return distance;
}

void _handle_sensor_triggers(train_position_info_t* tpi, sensor_triggers_t* triggers,uint32_t train_number, int32_t sensor_group, int32_t sensor_index) {
    //Check if we have a trigger set for this sensor and sensor_group.
    int32_t action_index = (sensor_group*8)+sensor_index;
    if(sensor_triggers_has_triggers(triggers, action_index)) {
        sensor_trigger_info_t* sti;
        //Act on the action related to the stop sensor

        while((sti = sensor_triggers_get(triggers,action_index)) != NULL){
            switch(sti->type) {
                case TRIGGER_STOP_AT:
                    tcs_train_set_speed(train_number, 0); 
                    break;
                case TRIGGER_STOP_AROUND:
                    send_term_debug_log_msg("Handling trigger: TRIGGER_STOP_AROUND at: %c%d", sensor_id_to_letter(action_index), sensor_id_to_number(action_index));
                    handle_train_stop_around_sensor(tpi,train_number,sti->byte1,sti->num1,false);
                    //_unset_sensor_trigger(triggers,sensor_group,sensor_index);
                    break;
                case TRIGGER_STOP_AROUND_USING_PATH:
                    send_term_debug_log_msg("Handling trigger: TRIGGER_STOP_AROUND_USING_PATH at: %c%d", sensor_id_to_letter(action_index), sensor_id_to_number(action_index));
                    handle_train_stop_around_sensor(tpi,train_number,sti->byte1,sti->num1,true);
                    //_unset_sensor_trigger(triggers,sensor_group,sensor_index);
                    break;
                case TRIGGER_SET_SWITCH:
                    send_term_debug_log_msg("Handling trigger: TRIGGER_SET_SWITCH at: %c%d", sensor_id_to_letter(action_index), sensor_id_to_number(action_index));
                    handle_train_set_switch_direction(tpi, sti->num1, sti->byte1);
                    break;
                case TRIGGER_SET_SWITCH_AND_REVERSE:
                    send_term_debug_log_msg("Handling trigger: TRIGGER_SET_SWITCH_AND_REVERSE at: %c%d", sensor_id_to_letter(action_index), sensor_id_to_number(action_index));
                    handle_train_set_switch_and_reverse(tpi, train_number, sti->num1, sti->byte1, true);
                    break;
                case TRIGGER_NONE:
                    printf(COM2, "\e[s\e[60;40HAction Index: %d\e[u", action_index);
                    Delay(200);
                    ASSERT(0);
                    break;
                default:
                    ASSERT(0);
            }
            //If this needs to be put inside the switch above, then we need a different way of looping over so we don't go to infinite
            sensor_triggers_add_free_slot(triggers, sti);

        }         
    }
}

void train_conductor(void) {
    conductor_info_t conductor_info;
    int train_server_tid;

    //Get the conductor request from the train
    Receive(&train_server_tid, (char*)&conductor_info, sizeof(conductor_info_t));
    Reply(train_server_tid, (char*)NULL, 0);

    //Delay until the specified time
    Delay(conductor_info.delay);

    switch(conductor_info.command) {
        case CONDUCTOR_TRAIN_STOP:
            //Send the request to the train server
            tcs_train_set_speed(conductor_info.train_number, 0);
            //TODO might want to change this
            Delay(360); //Delay as long as it will probably take us to stop
            train_server_stopped_at_destination(train_server_tid);
            break;
        case CONDUCTOR_SET_SWITCH:
            tcs_switch_set_direction(conductor_info.num1, conductor_info.byte1);
            break;
        case CONDUCTOR_REVERSE_TRAIN:
            train_reverse(conductor_info.train_number);
            break;
        default:
            ASSERT(0);
    }

    //Die
    Exit();
}

void handle_set_stop_offset(train_position_info_t* train_position_info,int32_t mm_diff) {
    train_position_info->stopping_offset = mm_diff;
    send_term_heavy_msg(false,"Set stopping offset %d", mm_diff);
}

void handle_sensor_data(int16_t train_number, int16_t slot, int8_t* sensor_data, sensor_triggers_t* sensor_triggers,train_position_info_t* train_position_info)   {
    //every time we get a tick from the sensor data coming in, keep track of it.
    train_position_info->last_tick_time_seen =  *((uint32_t*)(sensor_data+12));

    if(train_position_info->last_sensor != NULL) {
        track_node* last_sensor_track_node = train_position_info->last_sensor;
        track_node* sensor_error_next_sensor = train_position_info->sensor_error_next_sensor;
        track_node* switch_error_next_sensor = train_position_info->switch_error_next_sensor;
        track_node** next_sensor = &train_position_info->next_sensor;

        int i,result;
        uint32_t distance;
        uint32_t time;
        uint32_t average_velocity=0;
        bool is_switch_error = false;

        if(*next_sensor == NULL) {
            return;
        }
        
        bool update_error_expected_time = false;
        uint32_t expected_group = (*next_sensor)->num / 8;
        uint32_t expected_index = (*next_sensor)->num % 8;
        uint32_t sensor_error_group = 0, sensor_error_index = 0;
        uint32_t switch_error_group = 0, switch_error_index = 0;

        if(sensor_error_next_sensor != NULL) {
            sensor_error_group = sensor_error_next_sensor->num / 8;
            sensor_error_index = sensor_error_next_sensor->num % 8;
        }

        if(switch_error_next_sensor != NULL && switch_error_next_sensor != *next_sensor) {
            switch_error_group = switch_error_next_sensor->num / 8;
            switch_error_index = switch_error_next_sensor->num % 8;
        }

        for(i = 0; i < 10; ++i) {

            //The condition for hitting the sensor that we are expecting next
            if((expected_group == i && (sensor_data[expected_group] & (1 << (7-expected_index))) != 0) ||
                (sensor_error_next_sensor != NULL && sensor_error_group == i && 
                    ((sensor_data[sensor_error_group] & (1 << (7 - sensor_error_index))) != 0)) ||
                (switch_error_next_sensor != NULL && switch_error_group == i && 
                    (sensor_data[switch_error_group] & (1 << (7 - switch_error_index))) != 0)) {
                //Make sure only our sensor is enabled

                //Get the timestamp from the sensor data
                time = *((uint32_t*)(sensor_data+12));

                if((sensor_error_next_sensor != NULL && sensor_error_group == i && 
                    (sensor_data[sensor_error_group] & (1 << (7 - sensor_error_index))) != 0)) {
                    *next_sensor = sensor_error_next_sensor;
                    update_error_expected_time = true;
                    expected_group = sensor_error_group;
                    expected_index = sensor_error_index;
                } else if((switch_error_next_sensor != NULL && switch_error_next_sensor != *next_sensor 
                    && switch_error_group == i && 
                    (sensor_data[switch_error_group] & (1 << (7 - switch_error_index))) != 0)) {
                    *next_sensor = switch_error_next_sensor;
                    expected_group = switch_error_group;
                    expected_index = switch_error_index;
                    update_error_expected_time = true;
                    is_switch_error = true;
                }                
            
                if(update_error_expected_time) {
                    //Distance between the last sensor and the one we just hit
                    distance = distance_between_track_nodes(last_sensor_track_node, *next_sensor,is_switch_error);
                    result = _train_position_get_av_velocity(train_position_info, last_sensor_track_node, *next_sensor, &average_velocity);
                    if(result < 0) {
                        train_position_info->next_sensor_estimated_time = 1;

                    } else {
                        train_position_info->next_sensor_estimated_time = train_position_info->ticks_at_last_sensor + (distance * 100) / average_velocity;
                    }
                    
                } else {
                    //Distance between the last sensor and the one we just hit
                    distance = distance_between_track_nodes(last_sensor_track_node, *next_sensor,false);
                }

                int32_t velocity = (distance * 100)/(time - train_position_info->ticks_at_last_sensor);

                result = _train_position_update_av_velocity(train_position_info,last_sensor_track_node,*next_sensor, velocity,&average_velocity);
                ASSERT(result>=0);

                _handle_sensor_triggers(train_position_info,sensor_triggers,train_number,expected_group,expected_index);
                
                //Send our time in mm / s
                send_term_update_velocity_msg(slot, velocity);

                //Currently sends the distance between the last 2 sensors that we just passed by, in mmticks_at_last_sensor
                //send_term_update_dist_msg(slot, distance );
                int32_t time_difference = time - train_position_info->next_sensor_estimated_time;
                
                //Update the error for the train on screen.
                send_term_update_err_msg(slot,(time_difference * ((int32_t)(average_velocity))) / 100 ); //Converts to mm distance

                handle_update_train_position_info(train_number, slot, train_position_info, time, average_velocity);
            }
        }
    }

    _handle_train_track_position_update(train_position_info);
    _handle_train_reservations(train_position_info);
}

void _handle_train_track_position_update(train_position_info_t* tpi){
    track_node* node_at_sensor = tpi->last_sensor; // The node where the sensor is located
    if(node_at_sensor == NULL) return;

    uint32_t    av_velocity = tpi->average_velocity;
    uint32_t    last_time   = Time( );
    uint32_t    last_node_time = tpi->ticks_at_last_sensor;
    uint32_t time = last_time - last_node_time;
    int offset_from_node = 0;
    offset_from_node = (av_velocity*time)/100;


    if(tpi->is_reversed){
        offset_from_node +=180;
    }else{
        offset_from_node +=50;
    }
    track_node* iterator_node = node_at_sensor;
    //iterate over the track until the length of the train is inside one of the nodes
    
    while(offset_from_node > get_track_node_length(iterator_node)){
    offset_from_node -= get_track_node_length(iterator_node);
        iterator_node = get_next_track_node(iterator_node);
    }
    tpi->leading_end_offset_in_node = offset_from_node;
    tpi->leading_end_node = iterator_node;

    //send_term_debug_log_msg("trackpos sens: %s tip: %s off %d",node_at_sensor->name,tpi->leading_end_node->name,tpi->leading_end_offset_in_node);
    
}
void _handle_train_reservations(train_position_info_t* tpi) {
    bool result;
    if(tpi->stopping_distance == NULL){
        return;  
    } 

    result = track_handle_reservations(&(tpi->reserved_node_queue) ,tpi->train_num, tpi->last_sensor, tpi->leading_end_offset_in_node, tpi->stopping_distance(tpi->speed, false));
    if(result == false){
       tcs_train_set_speed(tpi->train_num, 0); 
    }
}

bool handle_find_train(int16_t train, int16_t slot, int8_t* sensors, int8_t* initial_sensors, train_position_info_t* train_position_info) {
    //Find the train.
    int i;
    for(i = 0; i < 10; ++i) {
        if(sensors[i] != initial_sensors[i]) {
            tcs_train_set_speed(train, 0);
            int8_t diff = sensors[i] ^ initial_sensors[i];

            int j;
            for(j = 0; j < 8; j++) {
                if((diff & 0x1) != 0) {
                    //This is our index
                    break;
                }
                diff >>= 1;
            }

            uint32_t sensor = (i * 8) + (7 - j);
            train_position_info->last_sensor = tps_set_train_sensor(train, sensor);
            ASSERT(train_position_info->last_sensor != NULL);

            send_term_heavy_msg(false, "Found train %d at Sensor: %s!", train, train_position_info->last_sensor->name);
            update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)train_position_info->last_sensor->name));

            train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);
            update_terminal_train_slot_next_location(train, slot, (train_position_info->next_sensor == NULL) ? -1 
                : sensor_to_id((char*)(train_position_info->next_sensor->name)));

            if(train_position_info->next_sensor != NULL) {
                //Set the predicted nodes for error cases
                train_position_info->sensor_error_next_sensor = get_next_sensor(train_position_info->next_sensor);
                train_position_info->switch_error_next_sensor = get_next_sensor_switch_broken(train_position_info->last_sensor);

                //NOTE: if you put this back, make sure the sensors aren't null!
                //send_term_error_msg("If sensor broken: %s  If switch broken: %s", train_position_info.sensor_error_next_sensor->name, train_position_info.switch_error_next_sensor->name);
            } else {
                train_position_info->sensor_error_next_sensor = NULL;
                train_position_info->switch_error_next_sensor = NULL;
            }
            //Reserve this piece of track
            track_reserve_node(train_position_info->last_sensor, train_position_info->train_num);
            ASSERT(train_position_info->last_sensor->reserved_by == train_position_info->train_num);
            load_calibration(train,train_position_info);
            return true;
        }
    }

    return false;
}

void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num) {
    int index = sensor_num / 8;
    stop_sensors[index] |= (0x1 << (7 - ((int16_t)sensor_num % 8)));
}

void handle_update_train_position_info(int16_t train, int16_t slot, train_position_info_t* train_position_info, int32_t time, uint32_t average_velocity) {
    
    train_position_info->ticks_at_last_sensor = time;

    //Set our most recent sensor to the sensor we just hit.
    train_position_info->last_sensor = train_position_info->next_sensor;
    train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);

    //Update the terminal display
    update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)(train_position_info->last_sensor)->name));
            
    if(train_position_info->next_sensor != NULL) {
        update_terminal_train_slot_next_location(train, slot, sensor_to_id((char*)(train_position_info->next_sensor->name)));

        train_position_info->average_velocity = average_velocity;
        //TODO check for error here?
        uint32_t dist_to_next_sensor = distance_between_track_nodes(train_position_info->last_sensor, train_position_info->next_sensor,false);

        train_position_info->next_sensor_estimated_time = time + (dist_to_next_sensor * 100) / average_velocity;

        //Update the predicted nodes for error cases
        train_position_info->sensor_error_next_sensor = get_next_sensor(train_position_info->next_sensor);
        train_position_info->switch_error_next_sensor = get_next_sensor_switch_broken(train_position_info->last_sensor);

        //NOTE: if you put this back, make sure the sensors aren't null!
        //send_term_error_msg("If sensor broken: %s  If switch broken: %s", train_position_info->sensor_error_next_sensor->name, train_position_info->switch_error_next_sensor->name);
    } else { 
        train_position_info->sensor_error_next_sensor = NULL;
        train_position_info->switch_error_next_sensor = NULL;

        update_terminal_train_slot_next_location(train, slot, -1);
    }
}

int estimate_ticks_to_position(train_position_info_t* tpi,track_node* start_sensor, track_node* end_sensor,int mm_diff) {
    ASSERT(start_sensor->type == NODE_SENSOR);
    ASSERT(end_sensor->type == NODE_SENSOR);

    track_node* iterator_node = start_sensor,*prev_node;
    uint32_t time = 0, distance = 0;
    prev_node = iterator_node;
    uint32_t av_velocity=0;

    for(iterator_node = get_next_sensor(start_sensor); iterator_node != end_sensor && iterator_node != NULL; iterator_node = get_next_sensor(iterator_node)) {
            _train_position_get_av_velocity(tpi,prev_node,iterator_node,&av_velocity);
            distance = get_track_node_length(prev_node);
            time += (distance*100)/av_velocity; // time is in 1/100ths of second so mult by 100 to get on the level
            prev_node = iterator_node;
    }
    
    if(iterator_node == NULL) {
        //Track lead us to a dead end we can't estimate
        return -1;

    }

    if(mm_diff > 0) {
         _train_position_get_av_velocity(tpi,prev_node,iterator_node,&av_velocity);
    }//else use the last av_velocity, if that av_velocity was never set or is zero, we will shortcircuit

    if(av_velocity == 0) {
        return time;
    }

    time += (mm_diff * 100) / av_velocity;

    return time;
}

int estimate_ticks_to_distance(train_position_info_t* tpi,track_node* start_sensor, int distance, bool use_path) {
    ASSERT(start_sensor->type == NODE_SENSOR);
    if(distance <= 0) return 0;
    track_node* iterator_node = start_sensor,*prev_node;
    uint32_t time = 0,segment_dist=0;
    prev_node = iterator_node;
    uint32_t av_velocity=0;

    if(use_path){
        iterator_node = get_next_sensor_or_exit_using_path(get_path_iterator(tpi->current_path,start_sensor));
    }else{
        iterator_node = get_next_sensor_or_exit(start_sensor);
    }


    int result;
    send_term_debug_log_msg("ETTD sens: %s dist: %d",start_sensor->name,distance);
    while(distance > 0 && iterator_node != NULL) {

            if(iterator_node->type == NODE_EXIT){
                result = _train_position_get_prev_first_av_velocity(tpi,prev_node,&av_velocity);
            } else {
                result = _train_position_get_av_velocity(tpi,prev_node,iterator_node,&av_velocity);
            }

            if(result != 0) {
                send_term_debug_log_msg("Prev Node: %s Next Node: %s", prev_node->name, iterator_node->name);
                return result;
            }

            if(use_path){
                segment_dist = distance_between_track_nodes_using_path(get_path_iterator(tpi->current_path,prev_node), iterator_node);
            }else {
                segment_dist = distance_between_track_nodes(prev_node, iterator_node, false);
            }

            if(distance < segment_dist  ) {
                segment_dist = distance;
            }

            time += (segment_dist*100)/av_velocity; // time is in 1/100ths of second so mult by 100 to get on the level
            distance -= segment_dist;
            prev_node = iterator_node;

            if(use_path){
                iterator_node = get_next_sensor_or_exit_using_path(get_path_iterator(tpi->current_path,start_sensor));
            }else{
                iterator_node = get_next_sensor_or_exit(start_sensor);
            }
            send_term_debug_log_msg("time: %d",time);
    }

    if(distance != 0) {
        ASSERT(0);
        //Track lead us to a dead end we can't estimate
        return -2;
    }

    return time;
}

void handle_train_stop_around_sensor(train_position_info_t* tpi,int32_t train_number,int8_t sensor_num, int32_t mm_diff, bool use_path) {
    int time;
    int32_t distance;
    distance = _distance_to_send_stop_command(tpi,tpi->next_sensor, sensor_num,mm_diff,use_path);

    if(tpi->is_reversed) {
        distance -= tpi->reverse_offset;
    }

    tpi->expected_stop_around_sensor = sensor_num;

    //Delay(2000);
    ASSERT(distance>=0); // Given sensor was too late 

    //get stopping distance
    tpi->last_stopping_distance = tpi->stopping_distance(tpi->speed, false);
    //get time to that spot

    time = estimate_ticks_to_distance(tpi,tpi->next_sensor, distance,use_path);
    //Start a conducter
    send_term_debug_log_msg("Estimated ticks %d", time);
    tid_t conductor_tid = Create(TRAIN_CONDUCTOR_PRIORITY,train_conductor);
    conductor_info_t conductor_info;
    conductor_info.command = CONDUCTOR_TRAIN_STOP;
    conductor_info.delay = time;
    conductor_info.train_number = train_number;
    Send(conductor_tid,(char*)&conductor_info,sizeof(conductor_info_t),NULL,0);

    int result = 0;
    PUSH_BACK(tpi->conductor_tids, conductor_tid, result); //TODO figure out if this is necessary
}

void handle_train_set_switch_direction(train_position_info_t* tpi, int16_t switch_num, int16_t direction){
    int time;
    int32_t distance = 0;
    int16_t switch_node_num = switch_num;

    //We've got 4 switches with weird assignments, so we need to have a special case for them
    if(0x99 <= switch_node_num && switch_node_num <=0x9c){
        switch_node_num -= (153 - 19);
    }

    distance = dist_between_node_and_index_using_path(get_path_iterator(tpi->current_path, tpi->next_sensor), 80 + ((switch_node_num - 1) * 2));
    send_term_debug_log_msg("htssd from: %s to: %d dist: %d",tpi->next_sensor->name,80, distance);
    distance -= 300;

    //get time to that spot
    time = estimate_ticks_to_distance(tpi,tpi->next_sensor, distance, true);

    //Start a conducter
    tid_t conductor_tid = Create(TRAIN_CONDUCTOR_PRIORITY,train_conductor);

    conductor_info_t conductor_info;
    conductor_info.command = CONDUCTOR_SET_SWITCH;
    conductor_info.num1 = switch_num;
    conductor_info.byte1 = (direction == DIR_STRAIGHT) ? 'S' : 'C';
    conductor_info.delay = time;
    Send(conductor_tid,(char*)&conductor_info,sizeof(conductor_info_t),NULL,0);

    int result = 0;
    PUSH_BACK(tpi->conductor_tids, conductor_tid, result); //TODO figure out if this is necessary
}

void handle_train_set_switch_and_reverse(train_position_info_t* train_position_info, int32_t train_number, int16_t switch_number, int8_t switch_direction, bool use_path) {
    handle_train_set_switch_direction(train_position_info, switch_number, switch_direction);

    //This is the reversing code which will hopefully work soon
    int time;
    int32_t distance;
    distance = distance_between_track_nodes_using_path(get_path_iterator(train_position_info->current_path, train_position_info->last_sensor), get_next_track_node(train_position_info->last_sensor));

    if(train_position_info->is_reversed) {
        distance -= train_position_info->reverse_offset;
    }

    ASSERT(distance>=0); // Given sensor was too late 

    //get stopping distance
    train_position_info->last_stopping_distance = train_position_info->stopping_distance(train_position_info->speed, false);
    //get time to that spot

    time = estimate_ticks_to_distance(train_position_info, train_position_info->next_sensor, distance, use_path);

    send_term_debug_log_msg("Estimated ticks to switch and reverse: %d", time);

    //Start a conductor
    tid_t conductor_tid = Create(TRAIN_CONDUCTOR_PRIORITY,train_conductor);
    conductor_info_t conductor_info;
    conductor_info.command = CONDUCTOR_REVERSE_TRAIN;
    conductor_info.delay = time;
    conductor_info.train_number = train_number;
    Send(conductor_tid, (char*)&conductor_info, sizeof(conductor_info_t), NULL, 0);

    int result = 0;
    PUSH_BACK(train_position_info->conductor_tids, conductor_tid, result); //TODO figure out if this is necessar
}

int _train_position_update_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t V, uint32_t* av_out) {
    if(tpi->ok_to_record_av_velocities ==false ) {
        return 0; // currently shouldn't update av vels. So don't
    }
    ASSERT(from->type == NODE_SENSOR);
    ASSERT(to->type == NODE_SENSOR);
    int i,to_index;
    to_index = to->num;
    avg_velocity_t* av;
    int16_t speed_index = GET_SPEED_INDEX(tpi->speed);

    for(i = 0; i < MAX_AV_SENSORS_FROM; i ++) {
        av = &(tpi->average_velocities[to_index][i][speed_index]);
        //Emptiness is defined by having a null from member    
        if(av->from == NULL) {
            av->from = from;
        }

        av->average_velocity = ((av->average_velocity * av->average_velocity_count) + V)/(av->average_velocity_count + 1);
        av->average_velocity_count++;
        *av_out = av->average_velocity;
            
        return 0;
    }
    //We should never get here, we need to increase MAX_AV_SENSORS_FROM
    ASSERT(0);
    return -1;
}

int _train_position_get_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t* av_out) {
    ASSERT(from->type == NODE_SENSOR);
    ASSERT(to->type == NODE_SENSOR);    
    avg_velocity_t* av;
    int i,to_index;
    uint16_t speed_index = GET_SPEED_INDEX(tpi->speed);
    to_index = to->num;

    if(tpi->average_velocities[to_index][0][speed_index].from == NULL) {
        tpi->average_velocities[to_index][0][speed_index].from = from;
        tpi->average_velocities[to_index][0][speed_index].average_velocity = tpi->default_av_velocity[speed_index];
        *av_out = tpi->average_velocities[to_index][0][speed_index].average_velocity;
        return 0;
    }
    
    for(i = 0; i < MAX_AV_SENSORS_FROM; i ++) {
        av = &(tpi->average_velocities[to_index][i][speed_index]);

        if(av->from == from) {
            *av_out = av->average_velocity;
            return 0;
        }
    }
 
    return -1;
}

int _train_position_get_av_velocity_at_speed(train_position_info_t* tpi, track_node* from, track_node* to, int16_t speed, uint32_t* av_out) {
    ASSERT(from->type == NODE_SENSOR);
    ASSERT(to->type == NODE_SENSOR);    
    avg_velocity_t* av;
    int i,to_index;
    uint16_t speed_index = GET_SPEED_INDEX(speed);
    to_index = to->num;
    if(tpi->average_velocities[to_index][0][speed_index].from == NULL) {
        tpi->average_velocities[to_index][0][speed_index].from = from;
        tpi->average_velocities[to_index][0][speed_index].average_velocity = tpi->default_av_velocity[speed_index];
        *av_out = tpi->average_velocities[to_index][0][speed_index].average_velocity;
        return 0;
    }
    
    for(i = 0; i < MAX_AV_SENSORS_FROM; i ++) {
        av = &(tpi->average_velocities[to_index][i][speed_index]);
        if(av->from == from) {
            *av_out = av->average_velocity;
            return 0;
        }
    }
 
    return -1;
}

int _train_position_get_prev_first_av_velocity(train_position_info_t* tpi, track_node* node, uint32_t* av_out) {
    //Super hacky, ben you can take marks off for this lol :P but not really. but actually.
    // lololol its 2 in the morning
    ASSERT(node->type == NODE_SENSOR);
    avg_velocity_t* av;
    int to_index;
    uint16_t speed_index = GET_SPEED_INDEX(tpi->speed);
    to_index = node->num;
    av = &(tpi->average_velocities[to_index][0][speed_index]);
    *av_out = av->average_velocity;
 
    return 0;
}

void handle_goto_destination(train_position_info_t* train_position_info, sensor_triggers_t* triggers, int16_t train, int8_t sensor_num) {

    track_node* current_location = train_position_info->last_sensor;
    track_node* destination = get_sensor_node_from_num(current_location, sensor_num);

    train_position_info->destination = sensor_num;

    find_path(current_location, destination, train_position_info->current_path, &(train_position_info->path_length));

    int32_t distance_at_node[TRACK_MAX];

    send_term_debug_log_msg("Path Length: %d", train_position_info->path_length);

        //TODO change this from being hardcoded to taking an actual speed
    //Also, not sure why this can't be above...
    train_position_info->speed = 9;

    int i;
    for(i = 0; i < train_position_info->path_length; ++i) {
        send_term_debug_log_msg("Path[%d] = %s", i, train_position_info->current_path[i]->name);
    }

    for(i = 0; i < train_position_info->path_length - 1; ++i) {
        if(i == 0) {
            distance_at_node[i] = 0;
        } else {
            distance_at_node[i] = distance_at_node[i - 1] + get_track_node_length(train_position_info->current_path[i]);
        }

        //If the node is a branch, we'll need to set the branch to the right direction
        if(train_position_info->current_path[i]->type == NODE_BRANCH) {
            int send_switch_command_distance = distance_at_node[i] - 100;

            if(train_position_info->is_reversed) {
                send_switch_command_distance -= train_position_info->reverse_offset;
            }

            int sensor_before_distance = get_sensor_before_distance_using_path(get_path_iterator(train_position_info->current_path, current_location), send_switch_command_distance);

            if(sensor_before_distance < 0) {
                send_term_debug_log_msg("Branch: %s Distatnode: %d Send switch distance: %d", train_position_info->current_path[i]->name, distance_at_node[i-1], send_switch_command_distance);
                Delay(100);
                ASSERT(sensor_before_distance >= 0);
            }
            uint8_t Byte=0;

            if(train_position_info->current_path[i]->edge[DIR_STRAIGHT].dest == train_position_info->current_path[i+1]) {
                //switch_straight
                send_term_debug_log_msg("Branch %s is being set to straight Triggered at sensor: %d", train_position_info->current_path[i]->name, sensor_before_distance);
                Byte = DIR_STRAIGHT;

            } else if( train_position_info->current_path[i]->edge[DIR_CURVED].dest == train_position_info->current_path[i+1]) {
                //switch_curved
                send_term_debug_log_msg("Branch %s is being set to curved Triggered at sensor: %d", train_position_info->current_path[i]->name, sensor_before_distance);
                Byte = DIR_CURVED;
            } else {
                //Don't know how this would happen...
                ASSERT(0);
            }
            send_term_debug_log_msg("Setting trigger to switch at: %c%d", sensor_id_to_letter(sensor_before_distance), sensor_id_to_number(sensor_before_distance));
            sensor_triggers_set(triggers,sensor_before_distance,TRIGGER_SET_SWITCH, &Byte,&(train_position_info->current_path[i]->num));
        }

        //Check to see if we need to reverse
        if(train_position_info->current_path[i+1] == train_position_info->current_path[i]->reverse->edge[DIR_AHEAD].dest ||
            train_position_info->current_path[i+1] == train_position_info->current_path[i]->reverse->edge[DIR_CURVED].dest) {

            int distance = distance_at_node[i - 1] - train_position_info->stopping_distance(train_position_info->speed, false);
            int sensor_before_distance = get_sensor_before_distance_using_path(get_path_iterator(train_position_info->current_path, current_location), distance);

            ASSERT(sensor_before_distance != -1);

            int16_t direction;
            if(train_position_info->current_path[i+1] == train_position_info->current_path[i]->reverse->edge[DIR_AHEAD].dest) {
                direction = DIR_AHEAD;
            } else {
                direction = DIR_CURVED;
            }

            send_term_debug_log_msg("Node %s to %s requires a reverse", train_position_info->current_path[i]->name, train_position_info->current_path[i + 1]->name);
            send_term_debug_log_msg("Setting trigger to reverse at: %c%d", sensor_id_to_letter(sensor_before_distance), sensor_id_to_number(sensor_before_distance));
            sensor_triggers_set(triggers, sensor_before_distance, TRIGGER_SET_SWITCH_AND_REVERSE, (uint8_t*)&direction, &(train_position_info->current_path[i]->num));
        }
    } 

    send_term_debug_log_msg("Setting stop destination at node: %s", train_position_info->current_path[i]->name);
    _set_stop_around_trigger(train_position_info, triggers, sensor_num, 0, true);

    //Start the path
    tcs_train_set_speed(train, 9);
}

void handle_train_reversing(int16_t train, int8_t slot, train_position_info_t* train_position_info) {
    train_position_info->is_reversed = !train_position_info->is_reversed;
    train_position_info->last_sensor = train_position_info->next_sensor->reverse;
    train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);

    update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)(train_position_info->last_sensor->name)));

    if(train_position_info->next_sensor != NULL) {
        update_terminal_train_slot_next_location(train, slot, sensor_to_id((char*)(train_position_info->next_sensor->name)));

        //Update the predicted nodes for error cases
        train_position_info->sensor_error_next_sensor = get_next_sensor(train_position_info->next_sensor);
        train_position_info->switch_error_next_sensor = get_next_sensor_switch_broken(train_position_info->last_sensor);
    } else { 
        train_position_info->sensor_error_next_sensor = NULL;
        train_position_info->switch_error_next_sensor = NULL;

        update_terminal_train_slot_next_location(train, slot, -1);
    }
}

void handle_stopped_at_destination(int16_t train_number, int8_t slot, train_position_info_t* train_position_info) {
    send_term_debug_log_msg("Train %d is stopped at its destination", train_number);
    send_term_debug_log_msg("Last Num: %d Next Num: %d Expected Sensor Num: %d",train_position_info->last_sensor->num, train_position_info->next_sensor->num, train_position_info->expected_stop_around_sensor);

    if(train_position_info->last_sensor->num != train_position_info->expected_stop_around_sensor &&
        train_position_info->next_sensor->num == train_position_info->expected_stop_around_sensor) {
        //Our stopping distance stuff is accurate enough that we can assume we're really close to this sensor
        uint32_t average_velocity = 0;

        ASSERT(_train_position_get_av_velocity_at_speed(train_position_info, train_position_info->last_sensor, train_position_info->next_sensor, train_position_info->last_speed, &average_velocity) == 0);

        handle_update_train_position_info(train_number, slot, train_position_info, Time(), average_velocity);
        (void)_train_position_get_av_velocity_at_speed;
    }
}
