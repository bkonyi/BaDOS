#ifndef _TRAIN_SERVER_H_
#define _TRAIN_SERVER_H_
#include <common.h>
#include <track/track_node.h>
#include <track/track_data.h>
#include <trains/sensor_triggers.h>
#include <trains/train_server_types.h>
#include <ring_buffer.h>
#include <trains/track_reservation_server.h>
#include <trains/path_instructions.h>

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

    int16_t train_num;
    int16_t train_slot;

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

    //Position inside the node
    int leading_end_offset_in_node;
    track_node *leading_end_node;
    uint32_t last_tick_time_seen;
    reserved_node_queue_t reserved_node_queue;
    track_node *last_sensor_hit;
    bool reservation_halted;
//    int32_t acceleration_thousandths_mm_ticks;
    int32_t acceleration_at_max_thousandths_mm_ticks;
    int32_t acceleration_while_accel_thousandths_mm_ticks;
    int32_t acceleration_current_thousandths_mm_ticks;
    int32_t velocity_thousandths_mm_ticks;
    int32_t dist_from_last_sensor;
    track_node_data_t   train_front_location;
    track_node_data_t   train_back_location;
    track_node_data_t   train_sensor_location;
    bool    is_accelerating;
    bool    temp_printed_once;
    int32_t current_stopping_distance;
    uint32_t ticks_for_last_reservation_accel;
    // boolean value that will signify one the find command has finished so that sensor data doesn't come through and make a reseravtion before we can stake a claim.
    bool jesus_take_the_wheel; 
    bool performing_goto;

    uint32_t last_position_time;
    bool stopping;
    bool stopping_position_set;
    int32_t last_stopping_distance_in_res;

    //Information about our current path
    track_node* current_path[TRACK_MAX];
    int path_length;
    path_instructions_t instructions;
    int8_t destination;
    bool is_going_to_random_destinations;
    bool waiting_on_reverse;
    bool ready_to_recalculate_path;
    bool at_branch_after_reverse;
    bool received_reverse;
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
void train_server_goto_random_destinations(tid_t tid);

int train_server_set_accel(tid_t tid,int32_t accel1, int32_t accel2) ;
int train_server_set_deccel(tid_t tid,int32_t accel);
#endif // _TRAIN_SERVER_H_
