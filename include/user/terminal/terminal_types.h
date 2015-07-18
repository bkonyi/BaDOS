#ifndef _TERMINAL_TYPE_H_
#define _TERMINAL_TYPE_H_
#include <trains/train_server_types.h>
typedef enum {
    TERMINAL_UPDATE_CLOCK    =  1,
    TERMINAL_ECHO_INPUT      =  2,
    TERMINAL_TRAIN_COMMAND   =  3,
    TERMINAL_SWITCH_COMMAND  =  4,
    TERMINAL_REVERSE_COMMAND =  5,
    TERMINAL_COMMAND_ERROR   =  6,
    TERMINAL_QUIT            =  7,
    TERMINAL_UPDATE_SENSORS  =  8,
    TERMINAL_START_CTRL      =  9,
    TERMINAL_STOP_CTRL       = 10,
    TERMINAL_SET_TRACK       = 11,
    TERMINAL_STOP_TRAIN_ON_SWITCH_COMMAND = 12,
    TERMINAL_REGISTER_TRAIN  = 13,
    TERMINAL_INIT_TRAIN_SLOT = 14,
    TERMINAL_CLEAR_TRAIN_SLOT = 15,
    TERMINAL_FIND_TRAIN      = 16,
    TERMINAL_UPDATE_TRAIN_SLOT_SPEED = 17,
    TERMINAL_UPDATE_TRAIN_SLOT_CURRENT_LOCATION = 18,
    TERMINAL_UPDATE_TRAIN_SLOT_NEXT_LOCATION    = 19,
    TERMINAL_INIT_ALL_SWITCHES                  = 20,
    TERMINAL_UPDATE_TRAIN_VELO                  = 21,
    TERMINAL_UPDATE_TRAIN_DIST                  = 22,
    TERMINAL_UPDATE_TRAIN_ERROR                 = 23,
    TERMINAL_COMMAND_SUCCESS                    = 24,
    TERMINAL_COMMAND_HEAVY_MESSAGE       = 25,
    TERMINAL_DISPLAY_TRAIN_CALIBRATION   = 26,
    TERMINAL_DEBUG_LOG_ENTRY = 27,
    TERMINAL_UNKNOWN_COMMAND = -1

} terminal_command_t;



typedef struct {
    terminal_command_t command;
    int32_t            num1;
    int32_t            num2;
    char               byte1;
    int8_t             sensors[10]; //I really don't want to put this here...
    avg_velocity_t*** average_velocity_info;
} terminal_data_t;

#endif// _TERMINAL_TYPE_H_
