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
    TERMINAL_QUIT			 =  7,
    TERMINAL_UPDATE_SENSORS  =  8,
    TERMINAL_START_CTRL      =  9,
    TERMINAL_STOP_CTRL       = 10,
    TERMINAL_UNKNOWN_COMMAND = -1

} terminal_command_t;

typedef struct {
    terminal_command_t command;
    int32_t            num1;
    int32_t            num2;
    char               byte1;
    int8_t             sensors[10]; //I really don't want to put this here...
} terminal_data_t;

/**
 * @brief Responsible for handling screen output and drawing.
 * @details This task handles all of the UI for the system.
 */
void terminal_server(void);

/**
 * @brief Updates the on-screen clock to a given time.
 * @details Updates the on-screen clock to a given time.
 * 
 * @param ticks The time to be set on screen.
 */
void update_terminal_clock(int32_t ticks);

/**
 * @brief Updates the sensor array and recent sensors on screen.
 * @details Updates the sensor array and recent sensors by diffing against the sensor data
 *  from the previous sensor updates. Does not do anything if no sensor changes have occured since
 *  the last call to this method.
 * 
 * @param sensors An array of 10 bytes which represent the sensor states of the 80 track sensors.
 */
void update_terminal_sensors_display(int8_t* sensors);

#endif //__TERMINAL_H__
