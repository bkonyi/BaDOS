#ifndef __TRAIN_CONTROLLER_COMMANDER_H__
#define __TRAIN_CONTROLLER_COMMANDER_H__

#include <common.h>

/**
 * @brief The server responsible for issuing commands to the train controller.
 * @details This server is the sole user task responsible for communicating with the train controller. It also
 *  spawns a user task which issues requests and listens for sensor query responses.
 */
void train_controller_commander_server(void);

/**
 * @brief Sends a request to the train controller server to change a train speed.
 * @details Sends a request to the train controller server to change a train speed. This is a wrapper around
 *  a Send command to the train controller server, but should not block for very long.
 * 
 * @param train The train which is to have its speed set.
 * @param speed The speed to accelerate the train to between [0-14]
 * 
 * @return -1 if invalid train number, -2 if the speed is invalid, 0 otherwise.
 */
int tcs_train_set_speed(int8_t train, int8_t speed);

/**
 * @brief Sends a request to the train controller server to reverse the train.
 * @details Sends a request to the train controller server to reverse the train. This is a wrapper around
 *  a Send command to the train controller server, but should not block for very long. The train controller server
 *  first stops the train, delays for a period of time, sends the reverse command, and then reaccelerates to the original speed
 * 
 * @param train The train to reverse.
 * @return -1 if invalid train number, 0 otherwise
 */
int train_reverse(int8_t train);

/**
 * @brief Sends a request to the train controller server to change the direction of a switch.
 * @details Sends a request to the train controller server to change the direction of a switch. This is a wrapper around
 *  a Send command to the train controller server, but should not block for very long. The train controller server
 *  first activates the solenoid for the switch and then immediately disables it.
 * 
 * @param switch_num The number of the switch to be moved.
 * @param direction The direction to set the switch to. Can be one of C, c, S, or s.
 * 
 * @return -1 if the switch number is invalid, -2 if the direction is invalid, 0 otherwise.
 */
int tcs_switch_set_direction(int16_t switch_num, char direction);

/**
 * @brief Sends a requeste to the train controller server to turn on the controller.
 * @details Sends a requeste to the train controller server to turn on the controller.
 */
void start_controller(void);

/**
 * @brief Sends a requeste to the train controller server to turn off the controller.
 * @details Sends a requeste to the train controller server to turn off the controller.
 */
void stop_controller(void);

/**
 * @brief Registers a train
 * @details [long description]
 * 
 * @param train The train to be registered
 * @param slot The slot the train will occupy
 */
int register_train(int8_t train, int8_t slot);

/**
 * @brief Finds the initial position of a registered trains.
 * @details Moves registered train slowly, one at a time, in order to find their initial position on the track.
 */
void find_train(int16_t train);

int tcs_train_request_calibration_info(int8_t train);

int trigger_train_stop_on_sensor(int8_t train, int8_t sensor_num);
void tcs_initialize_track_switches(void);
int tcs_send_stop_around_sensor_msg(int16_t train,int8_t sensor_num, int32_t mm_diff);
int tcs_send_train_stop_offset_msg(int16_t train, int32_t mm_diff);
int tcs_goto_destination(int16_t train, int8_t sensor_num);
int tcs_speed_all_train(int8_t speed);

#endif //__TRAIN_CONTROLLER_COMMANDER_H__
