#ifndef _TRAIN_SERVER_H_
#define _TRAIN_SERVER_H_
#include <common.h>

typedef enum train_server_cmd_t {
	TRAIN_SERVER_INIT 					= 1,
	TRAIN_SERVER_SENSOR_DATA 			= 2,
	TRAIN_SERVER_SWITCH_CHANGED			= 3
} train_server_cmd_t;

typedef struct train_server_msg_t {
	train_server_cmd_t command;
    uint32_t num1;
} train_server_msg_t;

typedef struct train_server_sensor_msg_t {
	train_server_cmd_t command;
    char sensors[10];
} train_server_sensor_msg_t;

/**
 * @brief A server that is used to act as the controller for a train.
 * the train_servre is created in a sort of stem cell state. It won't be able
 * to function until it receives a message indicating which train it should be controlling.
 * @details [long description]
 */
void train_server(void);

#endif // _TRAIN_SERVER_H_
