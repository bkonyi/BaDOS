#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <common.h>

typedef enum {
    TERMINAL_UPDATE_CLOCK    =  1,
    TERMINAL_ECHO_INPUT      =  2,
    TERMINAL_TRAIN_COMMAND   =  3,
    TERMINAL_SWITCH_COMMAND  =  4,
    TERMINAL_REVERSE_COMMAND =  5,
    TERMINAL_COMMAND_ERROR   =  6,
    TERMINAL_BACKSPACE       =  7,
    TERMINAL_QUIT			 =  8,
    TERMINAL_UPDATE_SENSORS  =  9,
    TERMINAL_UNKNOWN_COMMAND = -1

} terminal_command_t;

typedef struct {
    terminal_command_t command;
    int32_t            num1;
    int32_t            num2;
    char               byte1;
    int8_t             sensors[10]; //I really don't want to put this here...
} terminal_data_t;

void terminal_server(void);
void update_terminal_clock(int32_t ticks);
void update_terminal_sensors_display(int8_t* sensors);

#endif //__TERMINAL_H__
