/* 
 * Disclaimer: Most of this code was written in the span of 5 days on very little sleep. 
 * There were some tight deadlines so we did whatever we had to do to get things done and working.
 * I (Ben) slept in the lab for a week and Dan barely slept at all, so please don't judge us too much!
 */

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

static void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, sensor_triggers_t* ,train_position_info_t* train_position_info); 
static bool handle_find_train(int16_t train, int16_t slot, int8_t* sensors, int8_t* initial_sensors, train_position_info_t* train_position_info);
static void _set_train_location(train_position_info_t* tpi, int16_t train, int8_t slot, track_node* sensor_node);

static void handle_update_train_position_info(int16_t train, int16_t slot, train_position_info_t* train_position_info, int32_t time,uint32_t);
static int _train_position_update_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t V, uint32_t* av_out);
static int _train_position_get_av_velocity(train_position_info_t* tpi, track_node* from, track_node* to, uint32_t* av);
static int _train_position_get_av_velocity_at_speed(train_position_info_t* tpi, track_node* from, track_node* to, int16_t speed, uint32_t* av_out);

static void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers,int16_t sensor_num) ;
static void handle_set_stop_offset(train_position_info_t* train_position_info,int32_t mm_diff);
static int _train_position_get_prev_first_av_velocity(train_position_info_t* tpi, track_node* node, uint32_t* av_out) ; 


static void handle_goto_destination(train_position_info_t* train_position_info, int16_t train_num, int8_t sensor_num);

static void handle_train_reversing(int16_t train, int8_t slot, train_position_info_t* train_position_info);
static void handle_stopped_at_destination(int16_t train_number, int8_t slot, train_position_info_t* train_position_info);
static void handle_goto_random_destinations(train_position_info_t* tpi, sensor_triggers_t* triggers);

static void _handle_train_track_position_update(train_position_info_t* tpi);
static check_result_t _check_train_instructions(train_position_info_t* tpi);
static check_result_t _check_stop_instruction(train_position_info_t* tpi, path_instruction_t* instruction);
static check_result_t _check_back_stop_instruction(train_position_info_t* tpi, path_instruction_t* instruction);
static check_result_t _check_switch_instruction(train_position_info_t* tpi, path_instruction_t* instruction);
static check_result_t _check_reverse_instruction(train_position_info_t* tpi, path_instruction_t* instruction);
static void _handle_train_reservations(train_position_info_t* tpi);
static void _train_server_send_speed(int16_t train, int16_t speed);

static void handle_set_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, int8_t sensor);

static void _train_server_send_speed(int16_t train, int16_t speed);
static void train_speed_courrier(void);

static void _train_server_set_switch(int16_t switch_num, int16_t direction);
static void train_switch_courrier(void);
static void _train_server_reverse(int16_t train);
static void train_reverse_courrier(void);
static void _handle_set_accel(train_position_info_t* tpi, int32_t accel1, int32_t accel2);
static void _handle_set_deccel(train_position_info_t* tpi, int32_t deccel);
static void _update_track_node_data(train_position_info_t* tpi, int32_t current_acceleration, int32_t dist_travelled);
static void _reverse_train_node_locations(train_position_info_t* tpi);
static void _set_train_node_locations(train_position_info_t* tpi, track_node* node, int32_t offset) ;
static bool _handle_check_train_reservations_with_switch(train_position_info_t* tpi, int switch_num);

#define END_INSTRUCTIONS() goto end_instructions
#define SET_SWITCHES(main, secondary) ((((int32_t)(main)) << 16) | (secondary))
#define GET_MAIN_SWITCH(switches) ((int16_t)((switches) >> 16))
#define GET_SECONDARY_SWITCH(switches) ((int16_t)(switches))

#define BRANCH_STOP_OFFSET 220
#define BRANCH_SWITCH_OFFSET 250
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
    TRAIN_SERVER_RECALCULATE_PATH       = 20,
    TRAIN_SERVER_SET_ACCEL              = 21,
    TRAIN_SERVER_SET_DECCEL             = 22,
    TRAIN_SERVER_GOTO_RANDOM_DESTINATIONS = 16
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
    tpi->last_tick_time_seen =0;
    tpi->leading_end_node = NULL;
    tpi->leading_end_offset_in_node =0;
    tpi->stopping_distance = NULL;
    tpi->last_sensor_hit = NULL;
    tpi->reservation_halted = false;
    tpi->jesus_take_the_wheel = false;
    tpi->last_position_time=0;
    tpi->stopping = true;
    tpi->last_stopping_distance_in_res = 0;
    tpi->waiting_on_reverse = false;
    tpi->ready_to_recalculate_path = false;
    tpi->at_branch_after_reverse = false;
    tpi->velocity_thousandths_mm_ticks = 0;
    tpi->dist_from_last_sensor=0;
    tpi->is_accelerating = false;
    tpi->temp_printed_once = false;
    tpi->current_stopping_distance = 0;
    tpi->is_going_to_random_destinations = false;
    tpi->ticks_for_last_reservation_accel = 0;
    tpi->received_reverse = false;
    tpi->performing_goto = false;
    tpi->train_sensor_location.node = NULL;
    tpi->train_front_location.node = NULL;
    tpi->train_back_location.node = NULL;
    
    path_instructions_clear(&tpi->instructions);
    
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
                            //send_term_update_dist_msg(train_slot, distance_estimation);  
                        }
                        
                        last_distance_update_time = new_time;
                    }

                    if(train_position_info.ready_to_recalculate_path) {
                        train_position_info.ready_to_recalculate_path = false;
                        handle_goto_destination(&train_position_info, train_number, train_position_info.destination);
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
                _train_server_send_speed(train_number, 2);
                break;
            case TRAIN_SERVER_REQUEST_CALIBRATION_INFO:
                Reply(requester, (char*)&train_position_info.average_velocities, sizeof(avg_velocity_t) * 80 * MAX_AV_SENSORS_FROM * MAX_STORED_SPEEDS);
                break;
            case TRAIN_SERVER_SET_SPEED:
                new_speed = train_server_message->num1;
                if(new_speed!= 0 && train_position_info.reservation_halted == true ) {
                    send_term_debug_log_msg("preventing other speeds from overriding reservation halt");
                    break;
                }
                
                send_term_debug_log_msg("TR %d Setting SPEED %d",train_position_info.train_num, new_speed);

                if(new_speed != 15 && new_speed != train_position_info.speed) {

                    if(new_speed < train_position_info.speed) {
                        train_position_info.is_under_over = false;
                    } else if(new_speed > train_position_info.speed) {
                        train_position_info.is_under_over = true;
                    } // Else leave the over under, we changed to the same speed

                    train_position_info.last_speed = train_position_info.speed;
                    train_position_info.speed  = new_speed;

                    if(new_speed == 0) {
                        train_position_info.ok_to_record_av_velocities = false;
                        train_position_info.stopping = true;
                    }else {
                        train_position_info.ok_to_record_av_velocities = true;
                        train_position_info.stopping = false;
                    }

                    train_position_info.is_accelerating = true;
                }

                if(new_speed == 0 && is_stopping_at_landmark) {
                    is_stopping_at_landmark = false;
                    int time_diff = Time() - train_position_info.ticks_at_last_sensor;
                    int distance = time_diff * train_position_info.average_velocity / 100;
                    send_term_heavy_msg(false, "Stopping train at %d.%dcm from %s Estimated Stopping Distance: %d", distance / 10, distance % 10, train_position_info.last_sensor->name, train_position_info.last_stopping_distance);
                }
                break;
            case TRAIN_SERVER_SET_STOP_OFFSET:
                handle_set_stop_offset(&train_position_info, train_server_message->num1);
                break;
            case TRAIN_SERVER_GOTO_DESTINATION:
                set_terminal_train_slot_destination(train_position_info.train_num, train_position_info.train_slot, train_server_message->num1);

                handle_goto_destination(&train_position_info, train_number, train_server_message->num1);
                break;
            case TRAIN_SERVER_STOPPED_AT_DESTINATION:
                handle_stopped_at_destination(train_number, train_slot, &train_position_info);
                break;
            case TRAIN_SERVER_SET_ACCEL:
                _handle_set_accel(&train_position_info,train_server_message->num1,train_server_message->num2);
                break; 
            case TRAIN_SERVER_SET_DECCEL:
                _handle_set_deccel(&train_position_info,train_server_message->num1);
                break; 
            case TRAIN_SERVER_SET_REVERSING:
                handle_train_reversing(train_number, train_slot, &train_position_info);
                train_position_info.received_reverse = true;
                if(train_position_info.waiting_on_reverse) {
                    train_position_info.ready_to_recalculate_path = true;
                    train_position_info.waiting_on_reverse = false;
                }
                track_intial_reservations(&(train_position_info.reserved_node_queue),train_position_info.train_num,&(train_position_info.train_sensor_location),&(train_position_info.train_sensor_location),100);

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
                    handle_goto_destination(&train_position_info, train_number, train_position_info.destination);
                }
                break;
            case TRAIN_SERVER_GOTO_RANDOM_DESTINATIONS:
                handle_goto_random_destinations(&train_position_info, &sensor_triggers);
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

void train_server_goto_random_destinations(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_GOTO_RANDOM_DESTINATIONS;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void _set_stop_on_sensor_trigger(sensor_triggers_t* triggers, int16_t sensor_num) {
    sensor_triggers_set(triggers, sensor_num, TRIGGER_STOP_AT, NULL, NULL);
}

void _print_train_node_locs(train_position_info_t* tpi) {
    send_term_debug_log_msg("tr$: %d front %s(%d) back %s(%d) sensor %s(%d)",tpi->train_num,tpi->train_front_location.node->name,tpi->train_front_location.offset,
        tpi->train_back_location.node->name,tpi->train_back_location.offset,
        tpi->train_sensor_location.node->name,tpi->train_sensor_location.offset);
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
                
                //Ensure that another train isn't on the sensor we are looking for
                if(!((*next_sensor)->reserved_by == -1 || (*next_sensor)->reserved_by == train_position_info->train_num)) {
                    break;
                }

                //Get the timestamp from the sensor data
                time = *((uint32_t*)(sensor_data+12));

                if(  sensor_error_next_sensor != NULL && sensor_error_group == i && 
                    (sensor_data[sensor_error_group] & (1 << (7 - sensor_error_index))) != 0) {
                    if(!(sensor_error_next_sensor->reserved_by == train_position_info->train_num || sensor_error_next_sensor->reserved_by == -1)){
                        break;
                    } else {
                        *next_sensor = sensor_error_next_sensor;
                        update_error_expected_time = true;
                        expected_group = sensor_error_group;
                        expected_index = sensor_error_index;
                    }
                } else if((switch_error_next_sensor != NULL && switch_error_next_sensor != *next_sensor 
                    && switch_error_group == i && 
                    (sensor_data[switch_error_group] & (1 << (7 - switch_error_index))) != 0)) {

                    if(!(switch_error_next_sensor->reserved_by == -1 || switch_error_next_sensor->reserved_by == train_position_info->train_num)){
                        //we can't own this piece of track
                        break;
                    } else {
                        *next_sensor = switch_error_next_sensor;
                        expected_group = switch_error_group;
                        expected_index = switch_error_index;
                        update_error_expected_time = true;
                        is_switch_error = true;
                    }
                }          

                train_position_info->last_sensor_hit =  *next_sensor;     
                train_position_info->dist_from_last_sensor = 0;
                _set_train_node_locations(train_position_info, *next_sensor,0);

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
}

int32_t dist_using_vat(int64_t v_i, int64_t a,int64_t time){
        return v_i*time + (a * time*(time))/2;
}
int32_t velocity_using_vat(int64_t v_i, int64_t a,int64_t time){
    return v_i + a*time;
}
int32_t dist_using_vva(int64_t v_i, int64_t v_f,int64_t a){
    return (v_f*v_f-v_i*v_i)/(2*a);
}

#define GRANULARITY 10000
void _handle_train_track_position_update(train_position_info_t* tpi){
    if(!tpi->jesus_take_the_wheel) return;
    track_node* node_at_sensor = tpi->last_sensor_hit; // The node where the sensor is located

    if(node_at_sensor == NULL) return;
    uint32_t    av_velocity = tpi->average_velocities[node_at_sensor->num][0][tpi->speed].average_velocity;
    
    if(tpi->last_position_time == 0){
        tpi->last_position_time = Time();
    }
    uint32_t    time_now   = Time();
    uint32_t time = time_now - tpi->last_position_time;
    tpi->last_position_time = time_now;
    int32_t dist_change_mm_thousandths; 
    int32_t current_acceleration = tpi->acceleration_current_thousandths_mm_ticks;

    if(tpi->stopping ){
        if(tpi->is_accelerating){
            dist_change_mm_thousandths = dist_using_vat(tpi->velocity_thousandths_mm_ticks,(-1)*current_acceleration,time);
            tpi->velocity_thousandths_mm_ticks = velocity_using_vat(tpi->velocity_thousandths_mm_ticks,(-1)*current_acceleration,time);
        } else {
            dist_change_mm_thousandths = 0;
            tpi->velocity_thousandths_mm_ticks = 0;
        }
        
        if(tpi->velocity_thousandths_mm_ticks < 0) {
            tpi->is_accelerating = false;
            tpi->velocity_thousandths_mm_ticks = 0;
            tpi->acceleration_current_thousandths_mm_ticks = tpi->acceleration_while_accel_thousandths_mm_ticks;
        }
    } else {
        if(tpi->is_accelerating){
            dist_change_mm_thousandths = dist_using_vat(tpi->velocity_thousandths_mm_ticks,current_acceleration,time);
            tpi->velocity_thousandths_mm_ticks = velocity_using_vat(tpi->velocity_thousandths_mm_ticks,current_acceleration,time);
        } else {
            dist_change_mm_thousandths =( av_velocity *time )*(GRANULARITY/100);
        }

        if((tpi->velocity_thousandths_mm_ticks) > ((av_velocity*(GRANULARITY/100)*80))/100){
            tpi->acceleration_current_thousandths_mm_ticks = tpi->acceleration_at_max_thousandths_mm_ticks;
            tpi->is_accelerating = false;
            tpi->velocity_thousandths_mm_ticks = av_velocity*(GRANULARITY/100);
        }
    }
    
    if(dist_change_mm_thousandths <0) dist_change_mm_thousandths = 0;
    
    ASSERT(dist_change_mm_thousandths >=0); // We should only ever be adding distance, if we are reversing this node should have flipped

    _update_track_node_data(tpi,current_acceleration,dist_change_mm_thousandths/GRANULARITY);

    send_term_update_dist_msg(tpi->train_slot, tpi->dist_from_last_sensor);
    send_term_update_velocity_msg(tpi->train_slot,tpi->velocity_thousandths_mm_ticks*(GRANULARITY/100));

    if(tpi->performing_goto && (_check_train_instructions(tpi) == CHECK_FAIL)){
            send_term_debug_log_msg("Path ran into problem: recalculating");
            handle_goto_destination(tpi,tpi->train_num,tpi->destination);
    }

    _handle_train_reservations(tpi);
}

void _set_train_node_locations(train_position_info_t* tpi, track_node* node, int32_t offset) {
    int front_offset_from_node = offset;
    int back_offset_from_node = offset;
    int sensor_offset_from_node = offset;

    if(tpi->is_reversed){
        front_offset_from_node+=170;
        back_offset_from_node-=50;
    } else {
        front_offset_from_node+=50;
        back_offset_from_node-=170;
    }

    tpi->train_front_location = track_get_node_location(node,front_offset_from_node);
    tpi->train_sensor_location = track_get_node_location(node,sensor_offset_from_node);
    tpi->train_back_location = track_get_node_location(node,back_offset_from_node);
}

void _reverse_train_node_locations(train_position_info_t* tpi){
    tpi->is_reversed = !tpi->is_reversed;
     track_flip_node_data(&(tpi->train_sensor_location));
    _set_train_node_locations(tpi,tpi->train_sensor_location.node,tpi->train_sensor_location.offset);
}

void _update_track_node_data(train_position_info_t* tpi, int32_t current_acceleration, int32_t dist_travelled){
    tpi->dist_from_last_sensor+= dist_travelled;
    int32_t dist_between_sensors = distance_between_track_nodes(tpi->last_sensor, tpi->next_sensor,false);
    if(tpi->dist_from_last_sensor > dist_between_sensors){
        tpi->dist_from_last_sensor =dist_between_sensors;
        _set_train_node_locations(tpi,tpi->last_sensor,dist_between_sensors);
    } else {
        _set_train_node_locations(tpi,tpi->train_sensor_location.node,tpi->train_sensor_location.offset + dist_travelled);
    }
  
    tpi->current_stopping_distance = dist_using_vva(tpi->velocity_thousandths_mm_ticks,0,(-1)*current_acceleration)/GRANULARITY;
    if(tpi->current_stopping_distance > tpi->stopping_distance(tpi->speed,false)){
        tpi->current_stopping_distance = tpi->stopping_distance(tpi->speed,false);
    }

    if(tpi->current_stopping_distance < 0) {
        send_term_debug_log_msg("ERROR: Stopping distance < 0. Velo: %d Accel: %d StopDist: %d", tpi->velocity_thousandths_mm_ticks, (-1) * current_acceleration, tpi->stopping_distance(tpi->speed,false));
        ASSERT(0);
    }
}

check_result_t _check_train_instructions(train_position_info_t* tpi) {
    path_instruction_t instruction = path_instruction_peek(&tpi->instructions);

    if(instruction.command == INVALID) {
        return CHECK_NEUTRAL;
    }
    check_result_t check_result = CHECK_FAIL; 
    while(instruction.command != INVALID) {
       switch(instruction.command) {
            case STOP:
                check_result = _check_stop_instruction(tpi, &instruction);
                break;
            case BACK_STOP:
                check_result = _check_back_stop_instruction(tpi, &instruction);
                break;
            case SWITCH:
                check_result = _check_switch_instruction(tpi, &instruction);
                break;
            case REVERSE:
                check_result = _check_reverse_instruction(tpi, &instruction);
                break;
            case DONE:
                //TODO check if we want to find another destination
                tpi->performing_goto = false;

                path_instruction_pop(&tpi->instructions);

                if(tpi->is_going_to_random_destinations) {
                    handle_goto_random_destinations(tpi, NULL);
                }

                END_INSTRUCTIONS();
                break;
            case INVALID:
                ASSERT(0);
                break;
            default: 
                ASSERT(0);
                break;
        }

        if(check_result == CHECK_SUCCESS) {
            path_instruction_pop(&tpi->instructions);
        } else if(check_result == CHECK_NEUTRAL) {
            END_INSTRUCTIONS(); 
        } else if(check_result == CHECK_FAIL) {
            return CHECK_FAIL;
        }
        instruction = path_instruction_peek(&tpi->instructions);
    }
    //succesfully iterated on all
    return CHECK_SUCCESS;
end_instructions:
    return CHECK_NEUTRAL;
}

check_result_t _check_stop_instruction(train_position_info_t* tpi, path_instruction_t* instruction) {
    if(tpi->current_stopping_distance == 0) {
        return CHECK_NEUTRAL;
    }

    track_node_data_t instruction_node = instruction->instruction_node;
    track_node_data_t front_of_train = tpi->train_front_location;
    track_node** iterator = get_path_iterator(tpi->current_path, front_of_train.node);

    if(iterator == NULL) {
        return CHECK_FAIL;
        return CHECK_FAIL;
        ASSERT(0);
    }

    uint32_t distance_between_nodes = distance_between_track_nodes_using_path(iterator, instruction_node.node);

    distance_between_nodes -= front_of_train.offset;
    distance_between_nodes +=  130; //5CM offset 

    if(distance_between_nodes <= tpi->current_stopping_distance) {
        _train_server_send_speed(tpi->train_num, 0);
        return CHECK_SUCCESS;
    }

    return CHECK_NEUTRAL;
}

check_result_t _check_back_stop_instruction(train_position_info_t* tpi, path_instruction_t* instruction) {
    if(tpi->current_stopping_distance == 0) {
        return CHECK_NEUTRAL;
    }

    track_node_data_t instruction_node = instruction->instruction_node;
    track_node_data_t back_of_train = tpi->train_back_location;
    track_node_data_t front_of_train = tpi->train_front_location;
    track_node** iterator = get_path_iterator(tpi->current_path, back_of_train.node);
    uint32_t distance_between_nodes;

    if(iterator == NULL) {
        return CHECK_FAIL;
        iterator = get_path_iterator(tpi->current_path, front_of_train.node);

        if(iterator == NULL) {
            Delay(100);
            send_term_debug_log_msg("NULL: Can't find %s or %s", back_of_train.node->name, front_of_train.node->name);
            Delay(100);
        }

        ASSERT(iterator != NULL);

        distance_between_nodes = distance_between_track_nodes_using_path(iterator, instruction_node.node);
        distance_between_nodes += (get_track_node_length(back_of_train.node) - back_of_train.offset);
    } else {
        distance_between_nodes = distance_between_track_nodes_using_path(iterator, instruction_node.node);
        distance_between_nodes -= back_of_train.offset;
    }

    distance_between_nodes +=  180; //18CM offset

    if(distance_between_nodes <= tpi->current_stopping_distance) {
        _train_server_send_speed(tpi->train_num, 0);
        return CHECK_SUCCESS;
    }

    return CHECK_NEUTRAL;
}

check_result_t _check_switch_instruction(train_position_info_t* tpi, path_instruction_t* instruction) {
    track_node* switch_node = instruction->instruction_node.node;
    
    if(switch_node->reserved_by == tpi->train_num ) {
        if(_handle_check_train_reservations_with_switch(tpi, instruction->switch_num)){
            _train_server_set_switch(instruction->switch_num, instruction->direction);
            return CHECK_SUCCESS; 
        } else {
            return CHECK_FAIL;
        }
    }

    return CHECK_NEUTRAL;
}

check_result_t _check_reverse_instruction(train_position_info_t* tpi, path_instruction_t* instruction) {
    if(tpi->received_reverse) {
        tpi->received_reverse = false;
        return CHECK_SUCCESS;
    } else if(tpi->velocity_thousandths_mm_ticks == 0) {
        tpi->waiting_on_reverse = true;
        tpi->received_reverse = false;
        tpi->reverse_path_start = instruction->instruction_node.node;
        _train_server_reverse(tpi->train_num);
    }

    return  CHECK_NEUTRAL;
}

void _handle_train_reservations(train_position_info_t* tpi) {
    if(!tpi->jesus_take_the_wheel) return;
    
    bool result;
    int cur_stop_dist;

    cur_stop_dist = tpi->current_stopping_distance;
    result = track_handle_reservations(&(tpi->reserved_node_queue) ,tpi->train_num, &(tpi->train_sensor_location), &(tpi->train_sensor_location),cur_stop_dist );
    uint32_t time = Time();

    if(result == true ) {
        if(tpi->velocity_thousandths_mm_ticks == 0 && tpi->reservation_halted  && (time - tpi->ticks_for_last_reservation_accel) > 50){
            tpi->reservation_halted = false;
            _train_server_send_speed(tpi->train_num, 10);
            tpi->ticks_for_last_reservation_accel = time;
        }
    } else {
        if(tpi->velocity_thousandths_mm_ticks != 0 && (time - tpi->ticks_for_last_reservation_accel) > 50){
            tpi->reservation_halted = true;
            tpi->ticks_for_last_reservation_accel = time;
            _train_server_send_speed(tpi->train_num, 0);
        }
    }
}

bool _handle_check_train_reservations_with_switch(train_position_info_t* tpi, int switch_num) {
    if(!tpi->jesus_take_the_wheel) return false;
    int cur_stop_dist;

    cur_stop_dist = tpi->current_stopping_distance;
   
    return track_check_reservations_with_switch(&(tpi->reserved_node_queue) ,tpi->train_num, &(tpi->train_sensor_location),cur_stop_dist,switch_num);
}

bool handle_find_train(int16_t train, int16_t slot, int8_t* sensors, int8_t* initial_sensors, train_position_info_t* train_position_info) {
    //Find the train.
    int i;
    for(i = 0; i < 10; ++i) {
        if(sensors[i] != initial_sensors[i]) {
            _train_server_send_speed(train, 0);
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
            ASSERT(current_location!= NULL);

            train_position_info->last_sensor_hit = current_location;
            
            _set_train_location(train_position_info, train, slot, current_location);

            update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)train_position_info->last_sensor->name));

            train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);
            update_terminal_train_slot_next_location(train, slot, (train_position_info->next_sensor == NULL) ? -1 
                : sensor_to_id((char*)(train_position_info->next_sensor->name)));

            send_term_heavy_msg(false, "Found train %d at Sensor: %s!", train, current_location->name);

            //Initialize our location information
            train_position_info->dist_from_last_sensor = 0;
            send_term_debug_log_msg("train %d pre reserving %s", train_position_info->train_num,current_location->name );
            _set_train_node_locations(train_position_info,train_position_info->last_sensor_hit,0);
            _print_train_node_locs(train_position_info);

            //Reserve this piece of track
           ASSERT( track_intial_reservations(&(train_position_info->reserved_node_queue),train_position_info->train_num,&(train_position_info->train_sensor_location),&(train_position_info->train_sensor_location),100));

            if(!(train_position_info->last_sensor->reserved_by == train_position_info->train_num)) {
                send_term_debug_log_msg( "ERROR %s Was already owned by %d",train_position_info->last_sensor->name,train_position_info->last_sensor->reserved_by);
                ASSERT(0); 
            }
            load_calibration(train,train_position_info);

            train_position_info->jesus_take_the_wheel = true;
            return true;
        }
    }
    
    return false;
}

void _set_train_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, track_node* sensor_node) {

    train_position_info->last_sensor = sensor_node;

    send_term_debug_log_msg("[SET_LOCATION] Last sensor now: %s", sensor_node->name);

    int sensor_index = sensor_node->index;

    if(sensor_index > 79) {
        sensor_index = -1;
    }

    update_terminal_train_slot_current_location(train, slot, sensor_index);

    train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);
    send_term_debug_log_msg("[SET_LOCATION] Next sensor: %s", (train_position_info->next_sensor == NULL) ? "N/A" : (train_position_info->next_sensor->name));

    update_terminal_train_slot_next_location(train, slot, (train_position_info->next_sensor == NULL) ? -1 
        : sensor_to_id((char*)(train_position_info->next_sensor->name)));

    send_term_debug_log_msg("[SET_LOCATION] Updated next location on terminal");

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
    //send_term_debug_log_msg("2Last sens set: %s",train_position_info->last_sensor->name );
    train_position_info->next_sensor = get_next_sensor(train_position_info->last_sensor);

    //Update the terminal display
    update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)(train_position_info->last_sensor)->name));
    train_position_info->train_sensor_location.node = train_position_info->last_sensor;
            
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
        //send_term_error_msg("If sensor broken: %s  If switch b@proken: %s", train_position_info->sensor_error_next_sensor->name, train_position_info->switch_error_next_sensor->name);
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
    uint32_t av_velocity = 0;

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

    if(use_path) {
        iterator_node = get_next_sensor_or_exit_using_path(get_path_iterator(tpi->current_path,start_sensor));
    } else {
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
        } else {
            segment_dist = distance_between_track_nodes(prev_node, iterator_node, false);
        }

        if(distance < segment_dist  ) {
            segment_dist = distance;
        }

        time += (segment_dist*100)/av_velocity; // time is in 1/100ths of second so mult by 100 to get on the level
        distance -= segment_dist;
        prev_node = iterator_node;

        if(use_path) {
            iterator_node = get_next_sensor_or_exit_using_path(get_path_iterator(tpi->current_path,start_sensor));
        } else {
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

void handle_goto_destination(train_position_info_t* train_position_info, int16_t train, int8_t sensor_num) {
    train_position_info->performing_goto = true;
    track_node* current_location = train_position_info->train_sensor_location.node;

    if(train_position_info->speed != 0) {
        _train_server_send_speed(train_position_info->train_num,0);
    } else if(train_position_info->velocity_thousandths_mm_ticks !=0) {
        return;
    }
    ASSERT(current_location != NULL);

    track_node* destination = get_sensor_node_from_num(current_location, sensor_num);

    train_position_info->destination = sensor_num;

    find_path(current_location, destination, train_position_info->current_path, &(train_position_info->path_length));

    int i;
        
    //Clear the instruction list
    path_instructions_clear(&train_position_info->instructions);

    for(i = 0; i < train_position_info->path_length - 1; ++i) {
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
            //Add switch instruction
            ASSERT(current_path_node->type == NODE_BRANCH || current_path_node->type == NODE_MERGE);
            send_term_debug_log_msg("Adding stop before branch");
            path_instructions_add_switch(&train_position_info->instructions, current_path_node, direction);
        } 

        if(current_path_node->type == NODE_MERGE && i != 0) {
            track_node* previous_node = train_position_info->current_path[i - 1];

            int8_t direction;
            if(reverse_ahead_node->reverse == previous_node) {
                direction = DIR_AHEAD;
            } else {
                direction = DIR_CURVED;
            }
            ASSERT(current_path_node->reverse->type == NODE_BRANCH || current_path_node->reverse->type == NODE_MERGE);
            path_instructions_add_switch(&train_position_info->instructions, current_path_node->reverse, direction);
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

            //Add back stop command
            path_instructions_add_back_stop(&train_position_info->instructions, current_path_node, 200);

            if(current_path_node->type == NODE_MERGE) {
                path_instructions_add_switch(&train_position_info->instructions, current_path_node, direction);
            } 

            if(current_path_node->edge[DIR_AHEAD].dest->type == NODE_MERGE) {
                if(current_path_node->edge[DIR_AHEAD].dest->reverse->edge + DIR_AHEAD == current_path_node->edge[DIR_AHEAD].reverse) {
                    direction = DIR_AHEAD;
                } else {
                    direction = DIR_CURVED;
                }

                path_instructions_add_switch(&train_position_info->instructions, current_path_node->edge[DIR_AHEAD].dest, direction);
            }

            path_instructions_add_reverse(&train_position_info->instructions, current_path_node);


            //TODO remove hard code
            _train_server_send_speed(train_position_info->train_num, 10);
            return;
        }
    }
    path_instructions_add_stop(&train_position_info->instructions, destination, 0);
    path_instructions_add_done(&train_position_info->instructions);

    //TODO remove hard code
    _train_server_send_speed(train_position_info->train_num, 10);
}

void handle_train_reversing(int16_t train, int8_t slot, train_position_info_t* train_position_info) {
    _reverse_train_node_locations(train_position_info);
    send_term_debug_log_msg("Handling a reverse");

    int new_offset = distance_between_track_nodes(train_position_info->last_sensor,train_position_info->next_sensor,false)-train_position_info->dist_from_last_sensor;
    track_node* temp_node = train_position_info->last_sensor;
    train_position_info->last_sensor = train_position_info->next_sensor->reverse;
    train_position_info->next_sensor = temp_node->reverse;

    train_position_info->dist_from_last_sensor = new_offset;
    send_term_update_dist_msg(slot,train_position_info->dist_from_last_sensor);

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

void handle_goto_random_destinations(train_position_info_t* tpi, sensor_triggers_t* triggers) {
    int16_t random_sensor;
    do {
        random_sensor = rand() % 80;
    } while(random_sensor == 0 ||
            random_sensor == 1 ||
            random_sensor == 13 ||
            random_sensor == 12 ||
            random_sensor == 14 ||
            random_sensor == 15 ||
            random_sensor == 12 ||
            random_sensor == 10 ||
            random_sensor == 23 ||
            random_sensor == 22 ||
            random_sensor == 25 ||
            random_sensor == 26 || 
            random_sensor == 24 ||
            random_sensor == 27 ||
            random_sensor == 35 ||
            random_sensor == 34 ||
            random_sensor == 9 ||
            random_sensor == 8 ||
            random_sensor == 7 ||
            random_sensor == 6);
    tpi->is_going_to_random_destinations = true;

    send_term_debug_log_msg("Going to random destination: %c%d", sensor_id_to_letter(random_sensor), sensor_id_to_number(random_sensor));

    set_terminal_train_slot_destination(tpi->train_num, tpi->train_slot, random_sensor);

    handle_goto_destination(tpi, tpi->train_num, random_sensor);
}


void handle_set_location(train_position_info_t* train_position_info, int16_t train, int8_t slot, int8_t sensor) {
    track_node* current_sensor = tps_set_train_sensor(train, sensor);

    send_term_heavy_msg(false, "Found train %d at Sensor: %s!", train, current_sensor->name);

    _set_train_location(train_position_info, train, slot, current_sensor);

    load_calibration(train, train_position_info);
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

void _train_server_set_switch(int16_t switch_num, int16_t direction) {
    tid_t tid = CreateName(31, train_switch_courrier, "TRAIN_SWITCH_COURRIER");

    int16_t train_info[2];
    train_info[0] = switch_num;
    train_info[1] = direction;

    Send(tid, (char*)train_info, sizeof(int16_t) * 2, (char*)NULL, 0);
}

void train_switch_courrier(void) {
    tid_t train_server_tid;
    int16_t train_info[2];
    Receive(&train_server_tid, (char*)train_info, sizeof(int16_t) * 2);
    Reply(train_server_tid, (char*)NULL, 0);

    tcs_switch_set_direction(train_info[0], (train_info[1] == DIR_AHEAD) ? 'S' : 'C');

    Exit();
}

void _train_server_reverse(int16_t train) {
    tid_t tid = CreateName(31, train_reverse_courrier, "TRAIN_REVERSE_COURRIER");

    int16_t train_info;
    train_info = train;

    Send(tid, (char*)&train_info, sizeof(int16_t), (char*)NULL, 0);
}

void train_reverse_courrier(void) {
    tid_t train_server_tid;
    int16_t train_info;
    Receive(&train_server_tid, (char*)&train_info, sizeof(int16_t));
    Reply(train_server_tid, (char*)NULL, 0);

    train_reverse_immediately(train_info);

    Exit();
}

int train_server_set_accel(tid_t tid,int32_t accel1, int32_t accel2) {
    if(accel1 < 0 || accel2 < 0) return -1;
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_SET_ACCEL;
    msg.num1 = accel1;
    msg.num2 = accel2;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
    return 0;
}

void _handle_set_accel(train_position_info_t* tpi, int32_t accel1, int32_t accel2) {
    if(accel1 == 0 && accel2 == 0) {
        send_term_debug_log_msg("Set tr %d accel while acc: %d, while at max: %d", tpi->train_num,tpi->acceleration_while_accel_thousandths_mm_ticks,tpi->acceleration_at_max_thousandths_mm_ticks);
        return;
    }

    tpi->acceleration_while_accel_thousandths_mm_ticks = accel1;
    tpi->acceleration_at_max_thousandths_mm_ticks = accel2;
}

int train_server_set_deccel(tid_t tid,int32_t accel) {
    if(accel < 0) return -1;
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_SET_DECCEL;
    msg.num1 = accel;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
    return 0;
}
void _handle_set_deccel(train_position_info_t* tpi, int32_t deccel) {
    (void)tpi;
    (void)deccel;
}

