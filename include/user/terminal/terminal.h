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

/**
 * @brief Sets a train status slot to display the train number and place ?? values in other fields
 * @details Sets a train status slot to display the train number and place ?? values in other fields.
 * Used when initially registering a train or moving a train to a different slot
 * 
 * @param train The train number to have information displayed for
 * @param slot The slot for information to be shown in. Valid values: [1-6]
 */
void initialize_terminal_train_slot(int8_t train, int8_t slot);

/**
 * @brief Updates the speed for the train in a given slot.
 * @details Updates the speed for the train in a given slot.
 * 
 * @param train The train for which information on screen needs to be updated
 * @param slot The slot that the train currently occupies
 * @param speed The new speed to display on screen.
 */
void update_terminal_train_slot_speed(int8_t train, int8_t slot, int8_t speed);

/**
 * @brief [brief description]
 * @details [long description]
 * 
 * @param train [description]
 * @param slot [description]
 * @param sensor_location [description]
 */
void update_terminal_train_slot_current_location(int8_t train, int8_t slot, int8_t sensor_location);

/**
 * @brief [brief description]
 * @details [long description]
 * 
 * @param train [description]
 * @param slot [description]
 * @param sensor_location [description]
 */
void update_terminal_train_slot_next_location(int8_t train, int8_t slot, int8_t sensor_location);

/**
 * @brief Clears a train information slot on screen.
 * @details Clears a train information slot on screen.
 * 
 * @param slot The train information slot to clear.
 */
void clear_terminal_train_slot(int8_t slot);

void term_quit_msg (void);
void send_term_start_msg(void);
void send_term_stop_msg(void);
void send_term_quit_msg (void);
void send_term_find_msg(void);
void send_term_set_track_msg(char track);
void send_term_reverse_msg(uint32_t train_num);
void send_term_register_train_msg(int8_t train, int8_t slot);
void send_term_switch_msg(int32_t train_num, char state);
void send_term_train_msg(int32_t num, int32_t speed);
void send_term_error_msg(char* message, ...);
void send_term_initialize_track_switches(void);
void send_term_update_velocity_msg (uint32_t slot, uint32_t v);
void send_term_update_dist_msg (uint32_t slot, int32_t dist);
void send_term_update_err_msg(uint32_t slot, int32_t dist);
void send_term_update_err_msg(uint32_t slot, int32_t dist);
#endif //__TERMINAL_H__
