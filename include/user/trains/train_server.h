#ifndef _TRAIN_SERVER_H_
#define _TRAIN_SERVER_H_
#include <common.h>
#include <track/track_node.h>

#define MAX_AV_SENSORS_FROM 4
#define MAX_STORED_SPEEDS 7 //Can't make this any larger or else things just die...
#define GET_SPEED_INDEX(speed) (((speed) < 9) ? 0 : ((speed) - 8))

typedef enum train_server_cmd_t {
	TRAIN_SERVER_INIT 					= 1,
	TRAIN_SERVER_SENSOR_DATA 			= 2,
	TRAIN_SERVER_SWITCH_CHANGED			= 3,
    TRAIN_SERVER_REGISTER_STOP_SENSOR   = 4,
    TRAIN_SERVER_FIND_INIT_POSITION     = 5,
    TRAIN_SERVER_DIRECTION_CHANGE       = 6,
    TRAIN_SERVER_REQUEST_CALIBRATION_INFO = 7,
    TRAIN_SERVER_STOP_AROUND_SENSOR     = 8,
    TRAIN_SERVER_SET_SPEED              = 9,
    TRAIN_SERVER_SET_STOP_OFFSET        = 10,
    TRAIN_SERVER_GOTO_DESTINATION       = 11
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

typedef enum sensor_trigger_type_t {
    TRIGGER_NONE=1,
    TRIGGER_STOP_AT ,
    TRIGGER_STOP_AROUND
}sensor_trigger_type_t;

typedef struct sensor_trigger_info_t {
    sensor_trigger_type_t type;
    int32_t num1;
    int8_t byte1;
}sensor_trigger_info_t;

typedef struct sensor_triggers_t {
    int8_t  sensors[10];
    sensor_trigger_info_t action[80]; //each sensor gets a command
}sensor_triggers_t;

typedef struct {
    uint16_t average_velocity;
    uint16_t average_velocity_count;
    track_node* from;
} avg_velocity_t;


typedef struct train_position_info_t {
    int16_t speed;
    bool is_under_over;
	uint32_t ticks_at_last_sensor;
	track_node* last_sensor;
    uint32_t next_sensor_estimated_time;
    uint32_t average_velocity;
    avg_velocity_t average_velocities[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS];
    uint16_t (*stopping_distance)(uint16_t, bool);
    int16_t last_stopping_distance;

    track_node* next_sensor;
    track_node* sensor_error_next_sensor;
    track_node* switch_error_next_sensor;
    int32_t stopping_offset;
    tid_t conductor_tid;
    bool ok_to_record_av_velocities;
    uint32_t default_av_velocity[MAX_STORED_SPEEDS];
} train_position_info_t;

void _init_sensor_triggers(sensor_triggers_t* triggers);

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


void train_send_stop_around_sensor_msg(tid_t tid, int8_t sensor_num,int32_t mm_diff);

void train_request_calibration_info(tid_t tid, avg_velocity_t average_velocity_info[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS]);
void train_server_set_speed(tid_t tid, uint16_t speed);
void train_server_send_set_stop_offset_msg(tid_t tid, int32_t mm_diff);
void train_server_goto_destination(tid_t, int8_t sensor_num);

#endif // _TRAIN_SERVER_H_
