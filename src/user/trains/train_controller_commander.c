#include <trains/train_controller_commander.h>

#include <io/io.h>
#include <servers.h>
#include <syscalls.h>

#define REVERSE_DELAY_TICKS 200 //2000ms
#define CONTROLLER_DELAY()  Delay(5);

#define MAX_TRAIN_NUM 100 //TODO change this arbitrary value

#define MIN_SWITCH 1
#define MAX_SWITCH 256
#define SWITCH_DEACTIVATE 32
#define SWITCH_STRAIGHT   33
#define SWITCH_CURVE      34

#define MIN_SPEED        0
#define MAX_SPEED       14
#define REVERSE_COMMAND 15

typedef struct {
    int16_t train;
    int32_t reverse_time;
} reverse_delay_t;

static void train_reverse_delay_server(void);
static void send_reverse_delay(int server_id, int8_t train);

static void handle_train_set_speed(int8_t train, int8_t speed);
static void handle_train_reverse_begin(int8_t train);
static void handle_train_reverse_end(int8_t train, int8_t speed);
static void handle_switch_set_direction(int16_t switch_num, char direction);

void train_controller_commander_server(void) {
    bwprintf(COM2, "Creating train controller commander server...\r\n");
    int sending_tid;
    train_controller_data_t data;
    int8_t train_speeds[MAX_TRAIN_NUM + 1];

    int i;
    for(i = 0; i < MAX_TRAIN_NUM + 1; ++i) {
        train_speeds[i] = 0;
    }

    //TODO should change priority of this probably...
    int train_reverse_server_tid = Create(SCHEDULER_HIGHEST_PRIORITY - 1, train_reverse_delay_server);

    RegisterAs(TRAIN_CONTROLLER_SERVER);

    FOREVER {
        Receive(&sending_tid, (char*)&data, sizeof(train_controller_data_t));
        Reply(sending_tid, (char*)NULL, 0);

        switch(data.command) {
            case TRAIN_SET_SPEED:
                handle_train_set_speed((int8_t)data.var1, data.var2);
                train_speeds[data.var1] = data.var2;
                break;
            case TRAIN_REVERSE_BEGIN:
                //If the train isn't moving, we can just send the reverse command now
                if(train_speeds[data.var1] == 0) {
                    handle_train_set_speed((int8_t)data.var1, REVERSE_COMMAND);
                } else {
                    handle_train_reverse_begin((int8_t)data.var1);
                    send_reverse_delay(train_reverse_server_tid, (int8_t)data.var1);
                }
                break;
            case TRAIN_REVERSE_REACCEL:
                handle_train_reverse_end((int8_t)data.var1, train_speeds[data.var1]);
                break;
            case SWITCH_DIRECTION:
                handle_switch_set_direction(data.var1, data.var2);
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

void handle_train_set_speed(int8_t train, int8_t speed) {
    putc(COM1, speed);
    CONTROLLER_DELAY();//TODO I think we need this 50ms delay...
    putc(COM1, train);
}

void handle_train_reverse_begin(int8_t train) {
    handle_train_set_speed(train, 0);
}

void handle_train_reverse_end(int8_t train, int8_t speed) {
    handle_train_set_speed(train, REVERSE_COMMAND);
    CONTROLLER_DELAY();
    handle_train_set_speed(train, speed);
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
    CONTROLLER_DELAY();//TODO I think we need this 50ms delay...
    putc(COM1, switch_num);
    CONTROLLER_DELAY();//TODO I think we need this 50ms delay...
    putc(COM1, SWITCH_DEACTIVATE);
}
