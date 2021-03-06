#include <trains/train_controller_commander.h>

#include <io/io.h>
#include <servers.h>
#include <syscalls.h>
#include <terminal/terminal.h>
#include <task_priorities.h>
#include <trains/track_position_server.h>
#include <task_priorities.h>
#include <trains/train_server.h>
#include <terminal/terminal_debug_log.h>

#define REVERSE_DELAY_TICKS 450 //4500ms

#define MAX_TRAIN_NUM 100 //TODO change this arbitrary value

#define MIN_SWITCH 1
#define MAX_SWITCH 256
#define SWITCH_DEACTIVATE 32
#define SWITCH_STRAIGHT   33
#define SWITCH_CURVE      34

#define INVALID_SPEED   -1
#define MIN_SPEED        0
#define MAX_SPEED       14
#define REVERSE_COMMAND 15

#define MAX_REGISTERED_TRAINS     6
#define INVALID_REGISTERED_TRAIN -1
#define INVALID_SLOT             -1

#define QUERY_SENSORS_COMMAND 133

#define START_CONTROLLER_COMMAND 96
#define STOP_CONTROLLER_COMMAND  97

typedef enum {
    TRAIN_SET_SPEED=1 ,
    TRAIN_REVERSE_BEGIN,
    TRAIN_REVERSE_REACCEL,
    SWITCH_DIRECTION,
    SENSOR_QUERY_REQUEST, 
    START_CONTROLLER,
    STOP_CONTROLLER,
    TRAIN_REGISTER,
    FIND_TRAIN,
    TRAIN_TRIGGER_STOP_ON_SENSOR,
    TRAIN_CONTROLLER_UKNOWN_COMMAND,
    SWITCH_INITIALIZE_DIRECTIONS,
    REQUEST_CALIBRATION_INFO,
    TRAIN_TRIGGER_STOP_AROUND_SENSOR,
    SET_TRAIN_STOP_OFFSET,
    GOTO_LOCATION,
    SET_TRAIN_LOCATION,
    SET_TRAIN_ACCEL,
    SET_TRAIN_DECCEL,
    TRAIN_ALL_SET_SPEED,
    GOTO_RANDOM_LOCATION,
    ALL_GOTO_RANDOM_LOCATION,
    TRAIN_REVERSE_IMMEDIATE
} train_controller_command_t;

typedef struct {
    train_controller_command_t command;
    int16_t var1;
    int8_t  var2;
    uint32_t var3;
    uint32_t var4;
} train_controller_data_t;

typedef struct {
    int16_t train;
    int32_t reverse_time;
} reverse_delay_t;

typedef struct {
    int16_t speed;
    int16_t slot;
    bool is_reversing;
    tid_t server_tid;
} train_data_t;

static void train_reverse_delay_server(void);
static void send_reverse_delay(int server_id, int8_t train);

static void handle_train_set_speed(train_data_t* trains, int8_t train, int8_t speed);
static void handle_train_reverse_begin(train_data_t* trains, int8_t train);
static void handle_train_reverse_end(train_data_t* trains, int8_t train, int8_t speed);
static void handle_switch_set_direction(int16_t switch_num, char direction);

static void sensor_query_server(void);
static void handle_start_track_query(void);

static void handle_start_controller(void);
static void handle_stop_controller(void);

static void handle_train_register(train_data_t* trains, int16_t* train_slots, int8_t train, int8_t slot);

static void handle_find_train(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS], int16_t train);

static void handle_trigger_train_stop_on_sensor(train_data_t* trains, int8_t train, int8_t sensor_num);

static void handle_request_calibration_info(train_data_t* trains, int16_t train);

static void handle_initialize_track_switches(void);
static void handle_stop_around_sensor(train_data_t* trains, int8_t train, int8_t sensor_num,uint32_t mm_diff);
static void handle_train_stop_offset(train_data_t* trains,int32_t train_num,int32_t mm_diff);
static void handle_goto_location(train_data_t* trains, int32_t train_num, int8_t sensor_num);
static void handle_set_train_location(train_data_t* trains, int32_t train_num, int8_t sensor_num);
static void handle_set_all_speeds(train_data_t* trains,int8_t speed);
static void _handle_train_set_acceleration(train_data_t* trains, int8_t train_num,int32_t accel1,int32_t accel2);
static void _handle_train_set_decceleration(train_data_t* trains, int8_t train_num,int32_t deccel);
static void handle_goto_random_destinations(train_data_t* trains, int32_t train_num);
static void handle_all_goto_random_destinations(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS]);

void train_controller_commander_server(void) {
    int sending_tid;
    train_controller_data_t data;
    train_data_t trains[MAX_TRAIN_NUM + 1];
    int16_t registered_trains[MAX_REGISTERED_TRAINS];
    bool track_initialized = false;

    int i;
    for(i = 0; i <= MAX_TRAIN_NUM; ++i) {
        trains[i].speed = INVALID_SPEED;
        trains[i].slot = INVALID_SLOT;
        trains[i].is_reversing = false;
        trains[i].server_tid = -1;
    }

    for(i = 0; i < MAX_REGISTERED_TRAINS; ++i) {
        registered_trains[i] = INVALID_REGISTERED_TRAIN;
    }

    //TODO should change priority of this probably...
    //Create the server responsible for creating the delay when reversing trains
    int train_reverse_server_tid = CreateName(TRAIN_REVERSE_DELAY_SERVER_PRIORITY, train_reverse_delay_server, TRAIN_REVERSE_DELAY_SERVER);
    
    //Create the task responsible for requesting sensor querys and handling the responses
    CreateName(SENSOR_QUERY_SERVER_PRIORITY, sensor_query_server, SENSOR_QUERY_SERVER);

    //Since we turn the controller off when we start, we might as well turn it back on.
    handle_start_controller();

    RegisterAs(TRAIN_CONTROLLER_SERVER);
   
    FOREVER {   
        Receive(&sending_tid, (char*)&data, sizeof(train_controller_data_t));
       
        Reply(sending_tid, (char*)NULL, 0);
        switch(data.command) {
            case TRAIN_SET_SPEED:
                handle_train_set_speed(trains, (int8_t)data.var1, data.var2);
                break;
            case SET_TRAIN_ACCEL:
                _handle_train_set_acceleration(trains, (int8_t)data.var1, data.var3,data.var4);
                break;
            case SET_TRAIN_DECCEL:
                _handle_train_set_decceleration(trains, (int8_t)data.var1, data.var3);
                break;
            case TRAIN_REVERSE_BEGIN:
                //If the train isn't moving, we can just send the reverse command now
                
                if(trains[data.var1].speed == 0) {
                    handle_train_set_speed(trains, (int8_t)data.var1, REVERSE_COMMAND);
                } else {
                    handle_train_reverse_begin(trains, (int8_t)data.var1);
                    send_reverse_delay(train_reverse_server_tid, (int8_t)data.var1);
                }
                break;
            case TRAIN_REVERSE_IMMEDIATE:
                handle_train_set_speed(trains, (int8_t)data.var1, REVERSE_COMMAND);
                break;
            case TRAIN_REVERSE_REACCEL:
                handle_train_reverse_end(trains, (int8_t)data.var1, trains[data.var1].speed);
                break;
            case SWITCH_DIRECTION:
                handle_switch_set_direction(data.var1, data.var2);
                break;
            case SWITCH_INITIALIZE_DIRECTIONS:
                track_initialized = true;
                handle_initialize_track_switches();
                break;
            case SENSOR_QUERY_REQUEST:
                handle_start_track_query();
                break;
            case START_CONTROLLER:
                handle_start_controller();
                break;
            case STOP_CONTROLLER:
                handle_stop_controller();
                break;
            case TRAIN_REGISTER:
                if(track_initialized) {
                    send_term_register_train_msg(data.var1, data.var2);
                    handle_train_register(trains, registered_trains, data.var1, data.var2);
                } else {
                    send_term_heavy_msg(true, "Cannot register trains until the track is initialized");
                }
                break;
            case FIND_TRAIN:
                handle_find_train(trains, registered_trains, data.var1);
                break;
            case TRAIN_TRIGGER_STOP_ON_SENSOR:
                handle_trigger_train_stop_on_sensor(trains, data.var1, data.var2);
                break;
            case REQUEST_CALIBRATION_INFO:
                handle_request_calibration_info(trains, data.var1);
                break;
            case TRAIN_TRIGGER_STOP_AROUND_SENSOR:
                handle_stop_around_sensor(trains,data.var1,data.var2,data.var3);
                break;
            case SET_TRAIN_STOP_OFFSET:
                handle_train_stop_offset(trains,data.var1,data.var3);
                break;
            case GOTO_LOCATION:
                handle_goto_location(trains, data.var1, data.var2);
                break;
            case SET_TRAIN_LOCATION:
                handle_set_train_location(trains, data.var1, data.var2);
            case TRAIN_ALL_SET_SPEED:
                handle_set_all_speeds(trains,data.var2);//var2 has speed
                break;
            case GOTO_RANDOM_LOCATION:
                handle_goto_random_destinations(trains, data.var1);
                break;
            case ALL_GOTO_RANDOM_LOCATION:
                handle_all_goto_random_destinations(trains, registered_trains);
                break;
            default:
                printf(COM2, "Invalid train controller command!\r\n");
                ASSERT(0);
                break;
        }
    }
}

int tcs_train_set_speed(int8_t train, int8_t speed) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(speed > MAX_SPEED) {
        return -2;
    }

    train_controller_data_t data;
    data.command = TRAIN_SET_SPEED;
    data.var1 = (int16_t)train;
    data.var2 = speed;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

int tcs_speed_all_train(int8_t speed) {
    if(speed > MAX_SPEED) {
        return -2;
    }

    train_controller_data_t data;
    data.command = TRAIN_ALL_SET_SPEED;
    data.var2 = speed;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

void handle_set_all_speeds(train_data_t* trains,int8_t speed){
    int i;
    for(i = 0; i <= MAX_TRAIN_NUM; ++i) {
        if(trains[i].slot != INVALID_SLOT){
            handle_train_set_speed(trains,i,speed);
        }
    }
}

int train_reverse(int8_t train) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    }

    train_controller_data_t data;
    data.command = TRAIN_REVERSE_BEGIN;
    data.var1 = (int16_t)train;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

int train_reverse_immediately(int8_t train){
    if(train > MAX_TRAIN_NUM) {
        return -1;
    }

    train_controller_data_t data;
    data.command = TRAIN_REVERSE_IMMEDIATE;
    data.var1 = (int16_t)train;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

int tcs_switch_set_direction(int16_t switch_num, char direction) {
    if(switch_num < MIN_SWITCH || switch_num > MAX_SWITCH) {
        return -1;
    } else if(direction != 'C' && direction != 'c' &&
        direction != 'S' && direction != 's') {
        return -2;
    }

    train_controller_data_t data;
    data.command = SWITCH_DIRECTION;
    data.var1 = switch_num;
    data.var2 = direction;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

void tcs_initialize_track_switches(void) {
    train_controller_data_t data;
    data.command = SWITCH_INITIALIZE_DIRECTIONS;
    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
}

void handle_initialize_track_switches(void) {
    int i;
    for( i = 1; i < 19; i++ ) {
        if(is_valid_switch_number(i)){
            handle_switch_set_direction(i,'C');
        }
    }

    for( i = 0x99; i <= 0x9C; i++ ) {
        if(is_valid_switch_number(i)){
            handle_switch_set_direction(i,'C');
        }
    }
}

void start_controller(void) {
    train_controller_data_t data;
    data.command = START_CONTROLLER;
    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
}

void stop_controller(void) {
    train_controller_data_t data;
    data.command = STOP_CONTROLLER;
    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
}

int register_train(int8_t train, int8_t slot) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(slot <= 0 || slot > 6) {
        return -2;
    }

    train_controller_data_t data;
    data.command = TRAIN_REGISTER;
    data.var1 = train;
    data.var2 = slot;
    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

void find_train(int16_t train) {
    train_controller_data_t data;
    data.command = FIND_TRAIN;
    data.var1 = train;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
}

int trigger_train_stop_on_sensor(int8_t train, int8_t sensor_num) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(sensor_num >= 80) {
        return -2;
    }

    train_controller_data_t data;
    data.command = TRAIN_TRIGGER_STOP_ON_SENSOR;
    data.var1 = train;
    data.var2 = sensor_num;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

    return 0;
}

int tcs_send_stop_around_sensor_msg(int16_t train,int8_t sensor_num, int32_t mm_diff) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(sensor_num >= 80) {
        return -2;
    }

    train_controller_data_t data;
    data.command = TRAIN_TRIGGER_STOP_AROUND_SENSOR;
    data.var1 = train;
    data.var2 = sensor_num;
    data.var3 = mm_diff;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

int tcs_send_train_stop_offset_msg(int16_t train, int32_t mm_diff) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } 
    train_controller_data_t data;
    data.command = SET_TRAIN_STOP_OFFSET;
    data.var1 = train;
    data.var3 = mm_diff;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

int tcs_goto_destination(int16_t train, int8_t sensor_num) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(sensor_num >= 80) {
        return -2;
    }

    train_controller_data_t data;
    data.command = GOTO_LOCATION;
    data.var1 = train;
    data.var2 = sensor_num;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

void handle_train_stop_offset(train_data_t* trains,int32_t train_num,int32_t mm_diff) {
    train_server_send_set_stop_offset_msg(trains[(uint16_t)train_num].server_tid,mm_diff);
}

int set_train_location(int16_t train, int8_t sensor_num) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    } else if(sensor_num >= 80) {
        return -2;
    }

    train_controller_data_t data;
    data.command = SET_TRAIN_LOCATION;
    data.var1 = train;
    data.var2 = sensor_num;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

int tcs_goto_random_destinations(int16_t train) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    }

    train_controller_data_t data;
    data.command = GOTO_RANDOM_LOCATION;
    data.var1 = train;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

void tcs_all_goto_random_destinations(void) {
    train_controller_data_t data;
    data.command = ALL_GOTO_RANDOM_LOCATION;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
}

int tcs_train_request_calibration_info(int8_t train) {
    if(train > MAX_TRAIN_NUM) {
        return -1;
    }

    train_controller_data_t data;
    data.command = REQUEST_CALIBRATION_INFO;
    data.var1 = train;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

void train_reverse_delay_server(void) {
    int sending_tid;
    reverse_delay_t delay_request;

    train_controller_data_t reverse_command;
    reverse_command.command = TRAIN_REVERSE_REACCEL;

    FOREVER {
        Receive(&sending_tid, (char*)&delay_request, sizeof(reverse_delay_t));
        Reply(sending_tid, (char*)NULL, 0);

        //Wait until a certain timespan has passed since the train was reversed
        DelayUntil(delay_request.reverse_time);

        reverse_command.var1 = delay_request.train;
        Send(sending_tid, (char*)&reverse_command, sizeof(train_controller_data_t), (char*)NULL, 0);
    }
}

void send_reverse_delay(int server_id, int8_t train) {
    ASSERT(train <= MAX_TRAIN_NUM);
    reverse_delay_t delay_request;
    delay_request.train = train;
    delay_request.reverse_time  = Time() + REVERSE_DELAY_TICKS;

    Send(server_id, (char*)&delay_request, sizeof(reverse_delay_t), (char*)NULL, 0);
}

void sensor_query_server(void) {
    train_controller_data_t data;
    data.command = SENSOR_QUERY_REQUEST;

    int8_t sensors[SENSOR_MESSAGE_SIZE]; // to hold tick data

    FOREVER {
        Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

        //The priorities are set such that we have to be the first one scheduled after the 
        //clock server delegates it's time. This will 
        
        int i;
        for(i = 0; i < 10; ++i) {
            sensors[i] = getc(COM1);
        }
        uint32_t t = Time();
        memcpy(sensors+12,(char*)&t,sizeof(uint32_t));
        update_terminal_sensors_display(sensors);
        tps_send_sensor_data(sensors);
    }
}

void handle_train_set_speed(train_data_t* trains, int8_t train, int8_t speed) {
    putc(COM1, speed);
    putc(COM1, train);

    if(!trains[(uint16_t)train].is_reversing) {
        trains[(uint16_t)train].speed = speed;
        train_server_set_speed(trains[(uint16_t)train].server_tid, speed);
    }

    if(trains[(uint16_t)train].slot != INVALID_SLOT && speed != REVERSE_COMMAND) {
        update_terminal_train_slot_speed(train, trains[(uint16_t)train].slot, speed);
    }
    
    if(speed == REVERSE_COMMAND) {
        trains[(uint16_t)train].is_reversing = false;
        train_server_set_reversing(trains[(uint16_t)train].server_tid);
    }
}

void handle_train_reverse_begin(train_data_t* trains, int8_t train) {
    trains[(uint16_t)train].is_reversing = true;
    handle_train_set_speed(trains, train, 0);
}

void handle_train_reverse_end(train_data_t* trains, int8_t train, int8_t speed) {
    handle_train_set_speed(trains, train, REVERSE_COMMAND);
    if(speed != REVERSE_COMMAND){
        handle_train_set_speed(trains, train, speed);
    }else{
        send_term_debug_log_msg("YUP");
    }
}

void handle_switch_set_direction(int16_t switch_num, char direction) {
    int8_t direction_code;;

    if(direction == 'C' || direction == 'c') {
        direction_code = SWITCH_CURVE;
    } else if(direction == 'S' || direction == 's') {
        direction_code = SWITCH_STRAIGHT;
    } else {
        ASSERT(0);
        return;
    }

    tps_set_switch(switch_num,direction);

    putc(COM1, direction_code);
    putc(COM1, switch_num);
    putc(COM1, SWITCH_DEACTIVATE);

    send_term_switch_msg(switch_num, direction);
}

void handle_start_track_query(void) {
    putc(COM1, QUERY_SENSORS_COMMAND);
}

void handle_start_controller(void) {
    putc(COM1, START_CONTROLLER_COMMAND);
}

void handle_stop_controller(void) {
    putc(COM1, STOP_CONTROLLER_COMMAND);
}

void handle_train_register(train_data_t* trains, int16_t* train_slot, int8_t train, int8_t slot) {
    if(train_slot[(int16_t)slot] != INVALID_SLOT) {
        trains[train_slot[(int16_t)slot]].slot = INVALID_SLOT;
    }

    if(trains[(uint16_t)train].slot != INVALID_SLOT) {
        Destroy(trains[(int16_t)train].server_tid);
        clear_terminal_train_slot(trains[(int16_t)train].slot);
    }

    train_slot[(int16_t)slot] = train;
    trains[(int16_t)train].slot = slot;

    initialize_terminal_train_slot(train, slot);

    if(trains[(uint16_t)train].speed != INVALID_SPEED) {
        update_terminal_train_slot_speed(train, slot, trains[(uint16_t)train].speed);
    }

    tid_t tid = CreateName(TRAIN_SERVER_PRIORITY,train_server, "TRAIN_SERVER"); 
    trains[(int16_t)train].server_tid = tid;
    train_server_specialize(tid, train, slot);
}

void handle_find_train(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS], int16_t train) {    
    int i;
    for(i = 0; i < MAX_REGISTERED_TRAINS; ++i) {
        if(registered_trains[i] == train) {
            train_find_initial_position(trains[registered_trains[i]].server_tid);
        }
    }
}

void handle_request_calibration_info(train_data_t* trains, int16_t train) {
    if(trains[train].slot != INVALID_SLOT) {
        avg_velocity_t average_velocity_info[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS];
        train_request_calibration_info(trains[train].server_tid, average_velocity_info);
        print_train_calibration_info(train, average_velocity_info);
        //ASSERT(0);
    } else {
        send_term_heavy_msg(true, "Train: %d is not registered", train);
    }
}

void handle_trigger_train_stop_on_sensor(train_data_t* trains, int8_t train, int8_t sensor_num) {
    if(trains[(int16_t)train].server_tid != -1) {
        train_trigger_stop_on_sensor(trains[(int16_t)train].server_tid, sensor_num);
    }
}
void handle_stop_around_sensor(train_data_t* trains, int8_t train, int8_t sensor_num,uint32_t mm_diff) {
    if(trains[(int16_t)train].server_tid != -1) {
        train_send_stop_around_sensor_msg(trains[(int16_t)train].server_tid, sensor_num,mm_diff);
    }
}

void handle_goto_location(train_data_t* trains, int32_t train_num, int8_t sensor_num) {
    if(trains[train_num].server_tid != -1) {
        train_server_goto_destination(trains[train_num].server_tid, sensor_num);
        //set_terminal_train_slot_destination(train_num, trains[train_num].slot, sensor_num);
    }
}

void handle_set_train_location(train_data_t* trains, int32_t train_num, int8_t sensor_num) {
    if(trains[train_num].server_tid != -1) {
        train_server_set_location(trains[train_num].server_tid, sensor_num);
    }
}

int tcs_set_train_accel(int32_t train_num,int32_t accel1,int32_t accel2) {
    if(train_num > MAX_TRAIN_NUM) {
        return -1;
    } else if( accel1 < 0 || accel2 < 0) {
        return -2;
    }

    train_controller_data_t data;
    data.command = SET_TRAIN_ACCEL;
    data.var1 = train_num;
    data.var3 = accel1;
    data.var4 = accel2;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

void _handle_train_set_acceleration(train_data_t* trains, int8_t train_num,int32_t accel1,int32_t accel2){
    train_server_set_accel(trains[(int16_t)train_num].server_tid,accel1,accel2);
}

int tcs_set_train_deccel(int32_t train_num,int32_t deccel) {
    if(train_num > MAX_TRAIN_NUM) {
        return -1;
    } else if( deccel < 0) {
        return -2;
    }

    train_controller_data_t data;
    data.command = SET_TRAIN_DECCEL;
    data.var1 = train_num;
    data.var3 = deccel;

    Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);
    return 0;
}

void _handle_train_set_decceleration(train_data_t* trains, int8_t train_num,int32_t deccel){
    train_server_set_deccel(trains[(int16_t)train_num].server_tid,deccel);
}

void handle_goto_random_destinations(train_data_t* trains, int32_t train_num) {
    if(trains[train_num].server_tid != -1) {
        train_server_goto_random_destinations(trains[train_num].server_tid);
    }
}

void handle_all_goto_random_destinations(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS]) {
    int i;
    for(i = 0; i < MAX_REGISTERED_TRAINS; ++i) {
        if(registered_trains[i] != INVALID_REGISTERED_TRAIN) {
            train_server_goto_random_destinations(trains[registered_trains[i]].server_tid);
        }
    }
}

