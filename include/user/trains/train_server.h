#ifndef _TRAIN_SERVER_H_
#define _TRAIN_SERVER_H_
#include <common.h>
#include <track/track_node.h>
#include <track/track_data.h>
#include <trains/sensor_triggers.h>
#include <trains/train_server_types.h>
#include <ring_buffer.h>

#define MAX_AV_SENSORS_FROM 4
#define MAX_STORED_SPEEDS 7 //Can't make this any larger or else things just die...
#define GET_SPEED_INDEX(speed) (((speed) < 9) ? 0 : ((speed) - 8))

#define MAX_CONDUCTORS 32 //Arbitrary

CREATE_NON_POINTER_BUFFER_TYPE(conductor_buffer_t, int, MAX_CONDUCTORS);

typedef struct train_position_info_t {
    //Train information
    uint16_t train;

    //Calculates stopping distances for a given speed
    uint16_t (*stopping_distance)(uint16_t, bool);
    int16_t last_stopping_distance;
    int32_t stopping_offset;

    //Expected last sensor after stopping around sensor
    int8_t expected_stop_around_sensor;

    track_node* reverse_path_start;

    //Train orientation information
    bool is_reversed;
    int16_t reverse_offset;

    //Position information
    track_node* last_sensor;
    uint32_t ticks_at_last_sensor;
    track_node* next_sensor;
    uint32_t next_sensor_estimated_time;
    track_node* sensor_error_next_sensor;
    track_node* switch_error_next_sensor;
    
    //The live conductors for the train
    conductor_buffer_t conductor_tids;

    //Short move information
    //Takes in a speed and a distance to travel
    uint32_t (*short_move_time)(uint16_t, int16_t);

    //Speed information
    int16_t speed;
    int16_t last_speed;
    bool is_under_over;

    //Average velocity information
    bool ok_to_record_av_velocities;
    avg_velocity_t average_velocities[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS];
    uint32_t default_av_velocity[MAX_STORED_SPEEDS];
    uint32_t average_velocity;

    //Information about our current path
    track_node* current_path[TRACK_MAX];
    int path_length;
    int8_t destination;
    bool waiting_on_reverse;
    bool ready_to_recalculate_path;
    bool at_branch_after_reverse;

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
void train_send_sensor_update(tid_t tid, int8_t* sensors);
void train_send_stop_around_sensor_msg(tid_t tid, int8_t sensor_num,int32_t mm_diff);
void train_request_calibration_info(tid_t tid, avg_velocity_t average_velocity_info[80][MAX_AV_SENSORS_FROM][MAX_STORED_SPEEDS]);
void train_server_set_speed(tid_t tid, uint16_t speed);
void train_server_send_set_stop_offset_msg(tid_t tid, int32_t mm_diff);
void train_server_goto_destination(tid_t tid, int8_t sensor_num);
void train_server_set_reversing(tid_t tid);
void train_server_stopped_at_destination(tid_t tid);
void train_server_set_location(tid_t tid, int8_t sensor_num);

#endif // _TRAIN_SERVER_H_
