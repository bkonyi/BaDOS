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
#include <terminal/terminal_debug_log.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))


static void train_conductor(void);

static void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, sensor_triggers_t* ,train_position_info_t* train_position_info); 
static bool handle_find_train(int16_t train, int16_t slot, int8_t* sensors, int8_t* initial_sensors, train_position_info_t* train_position_info);
static void _set_train_location(train_position_info_t* tpi, int16_t train, int8_t slot, track_node* sensor_node);

static void handle_update_train_position_info(int16_t train, int16_t slot, train_position_info_t* train_position_info, int32_t time,uint32_t);
static int _train_position_update_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t V, uint32_t* av_out);
static int _train_position_get_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t* av);
static int _train_position_get_av_velocity_at_speed(train_position_info_t* tpi, track_node* from, track_node* to, int16_t speed, uint32_t* av_out);
static void handle_train_stop_around_sensor(train_position_info_t* tpi,int32_t train_number,int8_t sensor_num, int32_t mm_diff, bool use_path); 
static void handle_train_set_switch_direction(train_position_info_t* tpi, int16_t switch_num, int16_t direction);
static void handle_train_set_switch_and_reverse(train_position_info_t* train_position_info, int32_t train_number, int16_t switch_number, int8_t switch_direction, bool use_path);

static void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers,int16_t sensor_num) ;
static int _set_stop_around_trigger(train_position_info_t* tpi,sensor_triggers_t* triggers,int16_t sensor_num, int32_t mm_diff, bool use_path);
static void _handle_sensor_triggers(train_position_info_t* tpi, sensor_triggers_t* triggers,uint32_t train_number, int32_t sensor_group, int32_t sensor_index) ;
static void handle_set_stop_offset(train_position_info_t* train_position_info,int32_t mm_diff);
static int32_t _distance_to_send_stop_command(train_position_info_t* tpi,track_node* start_node,uint32_t destination_sensor_num, int32_t mm_diff,bool use_path) ;
static int _train_position_get_prev_first_av_velocity(train_position_info_t* tpi, track_node* node, uint32_t* av_out) ; 


static void handle_goto_destination(train_position_info_t* train_position_info, sensor_triggers_t* triggers, int16_t train_num, int8_t sensor_num);
static void _set_switch_change(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* current_path_node, int16_t distance, int8_t direction);
static void _set_reverse_and_switch(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* current_path_node, int16_t distance, int8_t direction);
static void _set_stop_around_location_using_path(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* destination);


static void handle_train_reversing(int16_t train, int8_t slot, train_position_info_t* train_position_info);
static void handle_stopped_at_destination(int16_t train_number, int8_t slot, train_position_info_t* train_position_info);

static void handle_set_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, int8_t sensor);

static void _train_server_recalculate_path_to_destination(tid_t tid);

static void _do_short_move(train_position_info_t* tpi, int16_t train, int16_t speed, int32_t delay, bool reverse, int32_t switch_num, int8_t direction);
static void _prepare_short_move_reverse(train_position_info_t* tpi, track_node* reverse_node, int32_t distance, int32_t switch_num, int8_t direction);
static void _train_server_send_speed(int16_t train, int16_t speed);
static void train_speed_courrier(void);


#define BRANCH_STOP_OFFSET 220
#define BRANCH_SWITCH_OFFSET 200
#define SHORT_MOVE_DEFAULT_SPEED 12
#define SWITCH_NONE -1
#define DIR_NONE -1

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
    TRAIN_SERVER_STOPPED_AT_DESTINATION = 13,
    TRAIN_SERVER_SET_LOCATION           = 14,
    TRAIN_SERVER_RECALCULATE_PATH       = 20
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
    CONDUCTOR_STOPPED_AT_DESTINATION = 3,
    CONDUCTOR_SHORT_MOVE_REVERSE = 4,
    CONDUCTOR_SHORT_MOVE = 5
} conductor_command_t;

typedef struct {
    conductor_command_t command;
    int32_t delay;
    int32_t train_number;
    int32_t num1;
    int32_t num2;
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
    tpi->waiting_on_reverse = false;
    tpi->ready_to_recalculate_path = false;
    tpi->at_branch_after_reverse = false;

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
    tps_add_train(train_number);

    train_position_info.train = train_number;
    int recalculate_counter = 0;

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
                train_position_info.waiting_on_reverse = false;
                handle_train_reversing(train_number, train_slot, &train_position_info);
                send_term_debug_log_msg("TRAIN_SERVER_SET_REVERSING");

                if(train_position_info.ready_to_recalculate_path) {
                    train_position_info.ready_to_recalculate_path = false;

                    send_term_debug_log_msg("TRAIN_SERVER_SET_REVERSING, READY TO RECALCULATE");

                    _set_train_location(&train_position_info, train_number, train_slot, train_position_info.reverse_path_start->reverse);

                    handle_goto_destination(&train_position_info, &sensor_triggers, train_number, train_position_info.destination);
                }

                break;
            case TRAIN_SERVER_SET_LOCATION:
                handle_set_location(&train_position_info, train_number, train_slot, train_server_message->num1);
                break;
            case TRAIN_SERVER_RECALCULATE_PATH:
                send_term_debug_log_msg("TRAIN_SERVER_RECALCULATE_PATH: %d Tid: %d", recalculate_counter++, requester);
                if(train_position_info.waiting_on_reverse) {
                    train_position_info.ready_to_recalculate_path = true;
                    send_term_debug_log_msg("TRAIN_SERVER_RECALCULATE_PATH, Waiting for reverse");
                } else {
                    _set_train_location(&train_position_info, train_number, train_slot, train_position_info.reverse_path_start->reverse);
                    send_term_debug_log_msg("TRAIN_SERVER_RECALCULATE_PATH, recalculating reverse path: %s!", train_position_info.last_sensor->name);
                    handle_goto_destination(&train_position_info, &sensor_triggers, train_number, train_position_info.destination);
                }
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

void train_server_set_location(tid_t tid, int8_t sensor_num) {
    train_server_msg_t msg;
    msg.num1 = sensor_num;
    msg.command = TRAIN_SERVER_SET_LOCATION;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void _train_server_recalculate_path_to_destination(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_RECALCULATE_PATH;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers, int16_t sensor_num) {
    sensor_triggers_set(triggers, sensor_num, TRIGGER_STOP_AT, NULL, NULL);
}

int _set_stop_around_trigger(train_position_info_t* tpi,sensor_triggers_t* triggers,int16_t sensor_num, int32_t mm_diff,bool use_path) {
    int32_t distance =0;
    int16_t sensor_to_trigger_at; 


    distance = _distance_to_send_stop_command(tpi,tpi->last_sensor,sensor_num,mm_diff, use_path);

    //If we're within 75cm of the stopping location after stopping distance, we can just short move to the destination
    if(distance <= 750) {

        //_distance_to_send_stop_command subtracts a stopping distance.
        //We add it back on in order to know how far the actual destination is
        if(tpi->speed != 0) {
            distance += tpi->stopping_distance(tpi->speed, false);
        }

        send_term_debug_log_msg("This needs to be a short move! Distance: %d", distance);
        int32_t ticks = tpi->short_move_time(tpi->speed, distance);

        send_term_debug_log_msg("Moving at speed: %d for %d ticks", 12, ticks);
        _do_short_move(tpi, tpi->train, tpi->speed, ticks, false, SWITCH_NONE, DIR_NONE);

        return 1;
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
        sensor_triggers_set(triggers,sensor_to_trigger_at,TRIGGER_STOP_AROUND, NULL, NULL);  
    }

    send_term_debug_log_msg("Stop around trigger set!");

    return 0;
}

int32_t _distance_to_send_stop_command(train_position_info_t* tpi,track_node* start_node,uint32_t destination_sensor_num, int32_t mm_diff,bool use_path) {
    int32_t distance = 0;

    track_node * destination_sensor = get_sensor_node_from_num(start_node,destination_sensor_num); 

    if(start_node == destination_sensor) {
        //TODO: MODIFY FOR USE WITH PATH
        ASSERT(0);
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

    send_term_debug_log_msg("Distance without stopping: %d", distance);

    if(tpi->speed != 0) {
        distance -= tpi->stopping_distance(tpi->speed, false);
    }

    if(tpi->is_reversed && !tpi->at_branch_after_reverse) {
        send_term_debug_log_msg("Added reverse offset!");
        distance -= tpi->reverse_offset;
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

    switch(conductor_info.command) {
        case CONDUCTOR_SHORT_MOVE_REVERSE:
        case CONDUCTOR_SHORT_MOVE:
            send_term_debug_log_msg("Starting short move... Speed: %d", conductor_info.num1);
            tcs_train_set_speed(conductor_info.train_number, conductor_info.num1);
            break;
        default:
            break;
    }

    //Delay until the specified time
    Delay(conductor_info.delay);

    switch(conductor_info.command) {
        case CONDUCTOR_TRAIN_STOP:
            send_term_debug_log_msg("CONDUCTOR_TRAIN_STOP stopping train");
            //Send the request to the train server
            tcs_train_set_speed(conductor_info.train_number, 0);
            //TODO might want to change this
            Delay(360); //Delay as long as it will probably take us to stop
            send_term_debug_log_msg("CONDUCTOR_TRAIN_STOP train stopped");
            train_server_stopped_at_destination(train_server_tid);
            break;
        case CONDUCTOR_SET_SWITCH:
            tcs_switch_set_direction(conductor_info.num1, conductor_info.byte1);
            break;
        case CONDUCTOR_REVERSE_TRAIN:
            send_term_debug_log_msg("CONDUCTOR_REVERSE_TRAIN stopping train");
            //Send the request to the train server
            tcs_train_set_speed(conductor_info.train_number, 0);
            //TODO might want to change this
            Delay(360); //Delay as long as it will probably take us to stop
            train_reverse(conductor_info.train_number);
            _train_server_recalculate_path_to_destination(train_server_tid);
            break;
        case CONDUCTOR_SHORT_MOVE_REVERSE:
            send_term_debug_log_msg("CONDUCTOR_SHORT_MOVE_REVERSE Stopping short move (reverse)...");
            tcs_train_set_speed(conductor_info.train_number, 0);
            Delay(conductor_info.delay); //Delay as long as it will probably take us to stop
            train_reverse(conductor_info.train_number);
            tcs_switch_set_direction(conductor_info.num2, (conductor_info.byte1 == DIR_STRAIGHT) ? 'S' : 'C');   
            _train_server_recalculate_path_to_destination(train_server_tid);
            break;
        case CONDUCTOR_SHORT_MOVE:
            send_term_debug_log_msg("CONDUCTOR_SHORT_MOVE Stopping short move (no reverse)...");
            tcs_train_set_speed(conductor_info.train_number, 0);
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

                //Currently sends the distance between the last 2 sensors that we just passed by, in mm
                //send_term_update_dist_msg(slot, distance );
                int32_t time_difference = time - train_position_info->next_sensor_estimated_time;
                
                //Update the error for the train on screen.
                send_term_update_err_msg(slot,(time_difference * ((int32_t)(average_velocity))) / 100 ); //Converts to mm distance

                handle_update_train_position_info(train_number, slot, train_position_info, time, average_velocity);
            }
        }
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
            track_node* current_location = tps_set_train_sensor(train, sensor);
            ASSERT(current_location != NULL);

            send_term_heavy_msg(false, "Found train %d at Sensor: %s!", train, current_location->name);

            _set_train_location(train_position_info, train, slot, current_location);

            load_calibration(train, train_position_info);
            return true;
        }
    }

    return false;
}

void _set_train_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, track_node* sensor_node) {

    train_position_info->last_sensor = sensor_node;

    int sensor_index = sensor_node->index;

    if(sensor_index > 79) {
        sensor_index = -1;
    }

    update_terminal_train_slot_current_location(train, slot, sensor_index);

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
    send_term_debug_log_msg("[ETTD] sens: %s dist: %d",start_sensor->name,distance);
    while(distance > 0 && iterator_node != NULL) {

            if(iterator_node->type == NODE_EXIT){
                result = _train_position_get_prev_first_av_velocity(tpi,prev_node,&av_velocity);
            } else {
                result = _train_position_get_av_velocity(tpi,prev_node,iterator_node,&av_velocity);
            }

            if(result != 0) {
                send_term_debug_log_msg("[ETTD] Prev Node: %s Next Node: %s", prev_node->name, iterator_node->name);
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
            send_term_debug_log_msg("[ETTD] time: %d",time);
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
    send_term_debug_log_msg("STOP_AROUND, Distance: %d", distance);

    tpi->expected_stop_around_sensor = sensor_num;

    if(distance < 0) {
        send_term_debug_log_msg("Too late to stop! Distance to stop: %d", distance);
        ASSERT(distance>=0); // Given sensor was too late 
    }

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
    //int16_t switch_node_num = switch_num;

    //We've got 4 switches with weird assignments, so we need to have a special case for them
    /*if(0x99 <= switch_node_num && switch_node_num <=0x9c){
        switch_node_num -= (153 - 19);
    }

    distance = dist_between_node_and_index_using_path(get_path_iterator(tpi->current_path, tpi->next_sensor), 80 + ((switch_node_num - 1) * 2));
    send_term_debug_log_msg("htssd from: %s to: %d dist: %d",tpi->next_sensor->name,80, distance);
    distance -= 300;*/

    //TODO remove this
    distance = 0;

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


    int16_t distance = distance_between_track_nodes_using_path(get_path_iterator(train_position_info->current_path, train_position_info->next_sensor), train_position_info->reverse_path_start) + BRANCH_STOP_OFFSET;

    send_term_debug_log_msg("[HANDLE_SW_RV] Distance to reverse: %d", distance);

    //This is the reversing code which will hopefully work soon
    int time;


    send_term_debug_log_msg("Last Sensor: %s, Next Sensor: %s", train_position_info->next_sensor->name, train_position_info->reverse_path_start->name);

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
    } else if(from->type != NODE_SENSOR) {
        //We don't track average velocities from nodes that aren't sensors
        return 0;
    }

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

    send_term_debug_log_msg("Path Length: %d", train_position_info->path_length);

    int i;
    for(i = 0; i < train_position_info->path_length; ++i) {
        send_term_debug_log_msg("Path[%d] = %s", i, train_position_info->current_path[i]->name);
    }


    //TODO change this from being hardcoded to taking an actual speed
    train_position_info->speed = 9;

    for(i = 0; i < train_position_info->path_length - 1; ++i) {
        track_node** path = train_position_info->current_path;
        track_node* current_path_node = train_position_info->current_path[i];
        track_node* next_path_node_straight = current_path_node->edge[DIR_STRAIGHT].dest;
        track_node* next_in_path_node = train_position_info->current_path[i + 1];
        track_node* reverse_ahead_node = train_position_info->current_path[i]->reverse->edge[DIR_AHEAD].dest;
        track_node* reverse_curve_node = train_position_info->current_path[i]->reverse->edge[DIR_CURVED].dest;

        if(current_path_node->type == NODE_BRANCH) {
            int8_t direction;
            if(next_in_path_node == next_path_node_straight) {
                direction = DIR_AHEAD;
            } else {
                direction = DIR_CURVED;
            }

            //Find the distance from the beginning of the path to the branch
            int16_t distance = distance_between_track_nodes_using_path(path, current_path_node);
            send_term_debug_log_msg("[GOTO (Switch)] Distance from start of path to %s: %d", current_path_node->name, distance);

            _set_switch_change(train_position_info, triggers, current_path_node, distance, direction);
        } 


        //Check for reverse path
        if(next_in_path_node == reverse_ahead_node || next_in_path_node == reverse_curve_node) {

            //Get the direction the reverse node takes from a branch
            int8_t direction;
            if(next_in_path_node == reverse_ahead_node) {
                direction = DIR_AHEAD;
            } else {
                direction = DIR_CURVED;
            }

            //Find the distance from the beginning of the path to the merge
            int16_t distance = distance_between_track_nodes_using_path(path, current_path_node);

            //Add the calibrated stop offset
            distance += train_position_info->stopping_offset;

            send_term_debug_log_msg("[GOTO (Reverse)] Distance from start of path to %s: %d", current_path_node->name, distance);

            _set_reverse_and_switch(train_position_info, triggers, current_path_node, distance, direction);
            return;
        }
    }

    //_set_stop_around_location_using_path(train_position_info, triggers, train_position_info->current_path[train_position_info->path_length - 1]);
    (void)_set_stop_around_location_using_path;

    (void)_train_server_send_speed;
    (void)_prepare_short_move_reverse;
}

void _set_switch_change(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* current_path_node, int16_t distance, int8_t direction) {
    track_node** path = tpi->current_path;

    //We're already stopped at the branch, so we can switch it now
    if(distance == 0) {
        send_term_debug_log_msg("[GOTO (Switch)] We're stopped at the branch. Switching it now.");
        tcs_switch_set_direction(current_path_node->num, (direction == DIR_AHEAD) ? 'S' : 'C');
        return;
    }

    //Subtract a train length so we don't switch too early
    distance -= BRANCH_SWITCH_OFFSET;

    //Find the sensor to trigger the switch to move
    int16_t trigger_sensor = get_sensor_before_distance_using_path(path, distance - tpi->stopping_distance(tpi->speed, false));


    //If we're already on the trigger sensor, we're probably not moving
    //so go ahead and set the switch now
    //If trigger_sensor < 0, we're too close or there's no sensor between us and the switch
    if(trigger_sensor < 0 || trigger_sensor == path[0]->index) {
        send_term_debug_log_msg("[GOTO (Switch)] Switch is too close. Switching it now.");
        tcs_switch_set_direction(current_path_node->num, (direction == DIR_AHEAD) ? 'S' : 'C');
        return;
    } else {
        //Set the trigger to switch the branch around when we hit the sensor
        send_term_debug_log_msg("[GOTO (Switch)] Setting trigger to %c at: %c%d", (direction == DIR_AHEAD) ? 'S' : 'C', sensor_id_to_letter(trigger_sensor), sensor_id_to_number(trigger_sensor));
        sensor_triggers_set(triggers, trigger_sensor, TRIGGER_SET_SWITCH, (uint8_t*)(&direction), &(current_path_node->num));
        return;
    }
}

void _set_reverse_and_switch(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* current_path_node, int16_t distance, int8_t direction) {
    track_node** path = tpi->current_path;

    //Add a buffer to stop past the merge to be ready to take the branch
    //Note: We might want to make this based on the length of the merge node
    distance += BRANCH_STOP_OFFSET;

    //NOTE: If we're currently reversed, we might need to account for the train being backwards
    /*if(train_position_info->is_reversed) {
        distance -= train_position_info->reverse_offset;
    }*/

    //Find the sensor we need to trigger at in order to stop and do the reverse in time
    int16_t trigger_sensor = get_sensor_before_distance_using_path(path, distance - tpi->stopping_distance(tpi->speed, false));
    
    //If distance is negative, we get -1
    //If we can't find a sensor, we get -2
    if(trigger_sensor < 0) {
        //This shouldn't happen
        send_term_debug_log_msg("%d", trigger_sensor);
        ASSERT(0);
    }

    //If the distance to the reverse is closer than 2 stopping distances, we probably
    //won't get up to speed. Do a short move.
    if((distance - tpi->stopping_distance(tpi->speed, false) < 0) ||
        trigger_sensor == path[0]->index) {
        send_term_debug_log_msg("[GOTO (Reverse)] Reverse path at %s requires a short move", current_path_node->name);

        //Figure out how long we need to move
        //We ignore the speed here, as we only move at speed 12 during short moves
        int32_t ticks = tpi->short_move_time(tpi->speed, distance);

        send_term_debug_log_msg("[GOTO (Reverse)] Moving at speed: %d for %d ticks for distance: %d", 12, ticks, distance);

        _do_short_move(tpi, tpi->train, 12, ticks, true, current_path_node->num, direction);

    } else {
        send_term_debug_log_msg("[GOTO (Reverse)] Triggering reverse at: %c%d", sensor_id_to_letter(trigger_sensor), sensor_id_to_number(trigger_sensor));

        //Set the trigger to start the switch+reverse process when we hit the sensor
        sensor_triggers_set(triggers, trigger_sensor, TRIGGER_SET_SWITCH_AND_REVERSE, (uint8_t*)(&direction), &(current_path_node->num));             
    }


    //This is where we're going to restart path finding from later
    tpi->reverse_path_start = current_path_node;
    tpi->waiting_on_reverse = true;

    _train_server_send_speed(tpi->train, tpi->speed);
}

void _set_stop_around_location_using_path(train_position_info_t* tpi, sensor_triggers_t* triggers, track_node* destination) {
    track_node** path = tpi->current_path;
    int16_t distance = distance_between_track_nodes_using_path(path, destination);

    send_term_debug_log_msg("[GOTO (Stop)] Distance from start of path to %s: %d", destination->name, distance);

    //Add the calibrated stop offset
    distance += tpi->stopping_offset;

    int16_t trigger_sensor = get_sensor_before_distance_using_path(path, distance - tpi->stopping_distance(tpi->speed, false));

    //If the distance to the reverse is closer than 2 stopping distances, we probably
    //won't get up to speed. Do a short move.
    if(trigger_sensor < 0 || (distance - tpi->stopping_distance(tpi->speed, false) < 0)) {
        send_term_debug_log_msg("[GOTO (Stop)] Stop path to %s requires a short move", destination->name);

        //Figure out how long we need to move
        //We ignore the speed here, as we only move at speed 12 during short moves
        int32_t ticks = tpi->short_move_time(tpi->speed, distance);

        send_term_debug_log_msg("[GOTO (Reverse)] Moving at speed: %d for %d ticks for distance: %d", 12, ticks, distance);
        _do_short_move(tpi, tpi->train, 12, ticks, false, SWITCH_NONE, DIR_NONE);

        return;
    }

    //TODO first arg shouldn't be null
    sensor_triggers_set(triggers, trigger_sensor, TRIGGER_STOP_AROUND_USING_PATH, (uint8_t*)(&destination->index), NULL);

    send_term_debug_log_msg("Stop around trigger set!");

    _train_server_send_speed(tpi->train, tpi->speed);

    return ;
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
        get_next_sensor(train_position_info->last_sensor)->num == train_position_info->expected_stop_around_sensor) {
        //Our stopping distance stuff is accurate enough that we can assume we're really close to this sensor
        uint32_t average_velocity = 0;

        ASSERT(_train_position_get_av_velocity_at_speed(train_position_info, train_position_info->last_sensor, train_position_info->next_sensor, train_position_info->last_speed, &average_velocity) == 0);

        handle_update_train_position_info(train_number, slot, train_position_info, Time(), average_velocity);
    }
}

void handle_set_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, int8_t sensor) {
    track_node* current_sensor = tps_set_train_sensor(train, sensor);

    send_term_heavy_msg(false, "Found train %d at Sensor: %s!", train, current_sensor->name);

    _set_train_location(train_position_info, train, slot, current_sensor);

    load_calibration(train, train_position_info);
}

void _do_short_move(train_position_info_t* tpi, int16_t train, int16_t speed, int32_t delay, bool reverse, int32_t switch_num, int8_t direction) {

    tid_t conductor_tid = Create(TRAIN_CONDUCTOR_PRIORITY, train_conductor);
    conductor_info_t conductor_info;
    conductor_info.command = reverse ? CONDUCTOR_SHORT_MOVE_REVERSE : CONDUCTOR_SHORT_MOVE;
    conductor_info.delay = delay;
    conductor_info.train_number = train;
    conductor_info.num1 = SHORT_MOVE_DEFAULT_SPEED; //TODO change this later
    conductor_info.num2 = reverse ? switch_num : SWITCH_NONE;
    conductor_info.byte1 = reverse ? direction : DIR_NONE;
    (void)speed;

    Send(conductor_tid,(char*)&conductor_info,sizeof(conductor_info_t),NULL,0);

    int result = 0;
    PUSH_BACK(tpi->conductor_tids, conductor_tid, result); //TODO figure out if this is necessary
}

void _prepare_short_move_reverse(train_position_info_t* tpi, track_node* reverse_node, int32_t distance, int32_t switch_num, int8_t direction) {
    //If we're reversed, take the length of the train into account in our distance
    /*if(tpi->is_reversed) {
        distance -= tpi->reverse_offset;
    }*/

    //Figure out how long we need to move
    int32_t ticks = tpi->short_move_time(tpi->speed, distance);

    send_term_debug_log_msg("Moving at speed: %d for %d ticks for distance: %d", 12, ticks, distance);

    //This is where we're going to restart path finding from later
    tpi->reverse_path_start = reverse_node;
    tpi->waiting_on_reverse = true;
    tpi->at_branch_after_reverse = true;

    _do_short_move(tpi, tpi->train, 12, ticks, true, switch_num, direction);
}

void _train_server_send_speed(int16_t train, int16_t speed) {
    tid_t tid = CreateName(31, train_speed_courrier, "TRAIN_SPEED_COURRIER");

    int16_t train_info[2];
    train_info[0] = train;
    train_info[1] = speed;

    Send(tid, (char*)train_info, sizeof(int16_t) * 2, (char*)NULL, 0);
}

void train_speed_courrier(void) {
    tid_t train_server_tid;
    int16_t train_info[2];
    Receive(&train_server_tid, (char*)train_info, sizeof(int16_t) * 2);
    Reply(train_server_tid, (char*)NULL, 0);

    tcs_train_set_speed(train_info[0], train_info[1]);

    Exit();
}
