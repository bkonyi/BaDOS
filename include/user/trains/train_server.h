#ifndef _TRAIN_SERVER_H_
#define _TRAIN_SERVER_H_
#include <common.h>
#include <track/track_node.h>

#define MAX_AV_SENSORS_FROM 4
typedef enum train_server_cmd_t {
	TRAIN_SERVER_INIT 					= 1,
	TRAIN_SERVER_SENSOR_DATA 			= 2,
	TRAIN_SERVER_SWITCH_CHANGED			= 3,
    TRAIN_SERVER_REGISTER_STOP_SENSOR   = 4,
    TRAIN_SERVER_FIND_INIT_POSITION     = 5,
    TRAIN_SERVER_DIRECTION_CHANGE       = 6,
    TRAIN_SERVER_REQUEST_CALIBRATION_INFO = 7
} train_server_cmd_t;

typedef struct train_server_msg_t {
	train_server_cmd_t command;
    uint32_t num1;
    uint32_t num2;
} train_server_msg_t;

typedef struct train_server_sensor_msg_t {
	train_server_cmd_t command;
    char sensors[SENSOR_MESSAGE_SIZE];
} train_server_sensor_msg_t;

typedef struct {
    uint32_t average_velocity;
    uint32_t average_velocity_count;
    track_node* from;
} avg_velocity_t;

typedef struct train_position_info_t {
	uint32_t ticks_at_last_sensor;
	track_node* last_sensor;
    uint32_t next_sensor_estimated_time;
    uint32_t average_velocity;
    avg_velocity_t average_velocities[80][MAX_AV_SENSORS_FROM];
    track_node* next_sensor;
    track_node* sensor_error_next_sensor;
    track_node* switch_error_next_sensor;
} train_position_info_t;

void train_position_info_init(train_position_info_t* tpi);
/**
 * @brief A server that is used to act as the controller for a train.
 * the train_servre is created in a sort of stem cell state. It won't be able
 * to function until it receives a message indicating which train it should be controlling.
 * @details [long description]
 */
void train_server(void);
void train_server_specialize(tid_t tid, uint32_t train_num, int8_t slot);
void train_trigger_stop_on_sensor(tid_t tid, int8_t sensor_num);
void train_find_initial_position(tid_t tid);
void train_request_calibration_info(tid_t tid, avg_velocity_t* average_velocity_info);
#endif // _TRAIN_SERVER_H_
