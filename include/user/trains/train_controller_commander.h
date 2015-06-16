#ifndef __TRAIN_CONTROLLER_COMMANDER_H__
#define __TRAIN_CONTROLLER_COMMANDER_H__

#include <common.h>

typedef enum {
    TRAIN_SET_SPEED,
    TRAIN_REVERSE_BEGIN,
    TRAIN_REVERSE_REACCEL,
    SWITCH_DIRECTION,
    SENSOR_QUERY_REQUEST, 
    TRAIN_CONTROLLER_UKNOWN_COMMAND
} train_controller_command_t;

typedef struct {
    train_controller_command_t command;
    int16_t var1;
    int8_t  var2;

} train_controller_data_t;

void train_controller_commander_server(void);
int train_set_speed(int8_t train, int8_t speed);
int train_reverse(int8_t train);
int switch_set_direction(int16_t switch_num, char direction);

#endif //__TRAIN_CONTROLLER_COMMANDER_H__
