#include <trains/train_controller_commander.h>

#include <io/io.h>
#include <servers.h>
#include <syscalls.h>
#include <terminal/terminal.h>
#include <task_priorities.h>
#include <trains/track_position_server.h>
#include <task_priorities.h>
#include <trains/train_server.h>

#define REVERSE_DELAY_TICKS 350 //3500ms

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
    TRAIN_SET_SPEED,
    TRAIN_REVERSE_BEGIN,
    TRAIN_REVERSE_REACCEL,
    SWITCH_DIRECTION,
    SENSOR_QUERY_REQUEST, 
    START_CONTROLLER,
    STOP_CONTROLLER,
    TRAIN_REGISTER,
    FIND_TRAINS,
    TRAIN_TRIGGER_STOP_ON_SENSOR,
    TRAIN_CONTROLLER_UKNOWN_COMMAND
} train_controller_command_t;

typedef struct {
    train_controller_command_t command;
    int16_t var1;
    int8_t  var2;

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

static void handle_find_trains(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS]);

static void handle_trigger_train_stop_on_sensor(train_data_t* trains, int8_t train, int8_t sensor_num);

void train_controller_commander_server(void) {
    int sending_tid;
    train_controller_data_t data;
    train_data_t trains[MAX_TRAIN_NUM + 1];
    int16_t registered_trains[MAX_REGISTERED_TRAINS];

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
            case TRAIN_REVERSE_BEGIN:
                //If the train isn't moving, we can just send the reverse command now
                if(trains[data.var1].speed == 0) {
                    handle_train_set_speed(trains, (int8_t)data.var1, REVERSE_COMMAND);
                } else {
                    handle_train_reverse_begin(trains, (int8_t)data.var1);
                    send_reverse_delay(train_reverse_server_tid, (int8_t)data.var1);
                }

                break;
            case TRAIN_REVERSE_REACCEL:
                handle_train_reverse_end(trains, (int8_t)data.var1, trains[data.var1].speed);
                break;
            case SWITCH_DIRECTION:
                handle_switch_set_direction(data.var1, data.var2);
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
                handle_train_register(trains, registered_trains, data.var1, data.var2);
                break;
            case FIND_TRAINS:
                handle_find_trains(trains, registered_trains);
                break;
            case TRAIN_TRIGGER_STOP_ON_SENSOR:
                handle_trigger_train_stop_on_sensor(trains, data.var1, data.var2);
                break;
            default:
                printf(COM2, "Invalid train controller command!\r\n");
                ASSERT(0);
                break;
        }
    }
}

int train_set_speed(int8_t train, int8_t speed) {
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

int switch_set_direction(int16_t switch_num, char direction) {
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

void find_trains(void) {
    train_controller_data_t data;
    data.command = FIND_TRAINS;

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

    int8_t sensors[10];

    FOREVER {
        Send(TRAIN_CONTROLLER_SERVER_ID, (char*)&data, sizeof(train_controller_data_t), (char*)NULL, 0);

        int i;
        for(i = 0; i < 10; ++i) {
            sensors[i] = getc(COM1);
        }

        update_terminal_sensors_display(sensors);
        tps_send_sensor_data(sensors);
    }
}

void handle_train_set_speed(train_data_t* trains, int8_t train, int8_t speed) {
    putc(COM1, speed);
    putc(COM1, train);

    if(!trains[(uint16_t)train].is_reversing) {
        trains[(uint16_t)train].speed = speed;
    }

    if(trains[(uint16_t)train].slot != INVALID_SLOT && speed != REVERSE_COMMAND) {
        update_terminal_train_slot_speed(train, trains[(uint16_t)train].slot, speed);
    }

    if(speed == REVERSE_COMMAND) {
        trains[(uint16_t)train].is_reversing = false;
    }

}

void handle_train_reverse_begin(train_data_t* trains, int8_t train) {
    trains[(uint16_t)train].is_reversing = true;

    handle_train_set_speed(trains, train, 0);
}

void handle_train_reverse_end(train_data_t* trains, int8_t train, int8_t speed) {
    handle_train_set_speed(trains, train, REVERSE_COMMAND);
    handle_train_set_speed(trains, train, speed);
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

    putc(COM1, direction_code);
    putc(COM1, switch_num);
    putc(COM1, SWITCH_DEACTIVATE);
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
    train_server_specialize(tid, train);
}
void handle_find_trains(train_data_t* trains, int16_t registered_trains[MAX_REGISTERED_TRAINS]) {
    //TODO update terminal status to respresent current state of find process
    
    int i;
    for(i = 1; i < MAX_TRAIN_NUM; ++i) {
        handle_train_set_speed(trains, i, 0);
    }

    for(i = 0; i < MAX_REGISTERED_TRAINS; ++i) {
        if(registered_trains[i] != INVALID_SLOT) {
            //TODO actually start find process for each registered train
        }
    }
}

void handle_trigger_train_stop_on_sensor(train_data_t* trains, int8_t train, int8_t sensor_num) {
    if(trains[(int16_t)train].server_tid != -1) {
        train_trigger_stop_on_sensor(trains[(int16_t)train].server_tid, sensor_num);
    }
}

