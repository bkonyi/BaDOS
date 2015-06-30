#include <trains/train_server.h>
#include <trains/track_position_server.h>
#include <trains/train_controller_commander.h>
#include <io/io.h>
#include <terminal/terminal.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))



static void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, int8_t* stop_sensors,track_node** last_sensor_track_node,train_position_info_t* train_position_info); 
static void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num);



void train_server(void) {
    //The bigger of the 2 should be the size we use for receive
    int message_size = (TRAIN_SERVER_MSG_SIZE > TRAIN_SERVER_SENSOR_MSG_SIZE?
                        TRAIN_SERVER_MSG_SIZE:
                        TRAIN_SERVER_SENSOR_MSG_SIZE);
	int requester;
	char message[message_size]; 
    int16_t train_number = -1;
    int16_t train_slot   = -1;
    int8_t  stop_sensors[10];

    bool finding_initial_position = false;
    bool initial_sensor_reading_received = false;
    int8_t finding_initial_sensor_state[10];

    int i;
    for(i = 0; i < 10; ++i) {
        stop_sensors[i] = 0;
    }

    train_position_info_t train_position_info;
    train_position_info_init(&train_position_info);

    track_node* last_sensor_track_node = NULL;


    //CURRENTLY A STEM CELL TRAIN,
    //need to obtain train info
    Receive(&requester,message, message_size);
    Reply(requester,NULL,0);
    if(((train_server_msg_t*)message)->command != TRAIN_SERVER_INIT) {
        //Our train hasn't been initialized
        ASSERT(0);
    }

    //
    tcs_switch_set_direction(8,'s');
    tcs_switch_set_direction(7,'s');    
    tcs_switch_set_direction(14, 's');

    train_number = ((train_server_msg_t*)message)->num1; 
    train_slot   = ((train_server_msg_t*)message)->num2;
    tps_add_train(train_number);

	FOREVER {
        //send_term_error_msg("Train: %d WAITING", train_number);
		Receive(&requester, message, message_size);
        Reply(requester, (char*)NULL, 0);

        switch (((train_server_msg_t*)message)->command) {
            case TRAIN_SERVER_SENSOR_DATA:
                if(!finding_initial_position) {

                    //Do calculations for our train.
                    handle_sensor_data(train_number, train_slot, ((train_server_sensor_msg_t*)message)->sensors, stop_sensors,&last_sensor_track_node,&train_position_info);
                } else {
                    if(!initial_sensor_reading_received) {
                        memcpy(finding_initial_sensor_state, ((train_server_sensor_msg_t*)message)->sensors, sizeof(int8_t) * 10);
                        initial_sensor_reading_received = true;
                    } else {
                        //Find the train.
                        int8_t* sensors = ((train_server_sensor_msg_t*)message)->sensors;
                        int i;
                        for(i = 0; i < 10; ++i) {
                            if(sensors[i] != finding_initial_sensor_state[i]) {
                                train_set_speed(train_number, 0);
                                int8_t diff = sensors[i] ^ finding_initial_sensor_state[i];

                                int j;
                                for(j = 0; j < 8; j++) {
                                    if((diff & 0x1) != 0) {
                                        //This is our index
                                        break;
                                    }
                                    diff >>= 1;
                                }

                                uint32_t sensor = (i * 8) + (7 - j);
                                (void)sensor;
                                last_sensor_track_node = tps_set_train_sensor(train_number, sensor);
                                ASSERT(last_sensor_track_node != NULL);
                                //send_term_error_msg("blah");
                                send_term_error_msg("Found train %d at Sensor: %s!", train_number, last_sensor_track_node->name);
                                update_terminal_train_slot_current_location(train_number, train_slot, sensor_to_id((char*)last_sensor_track_node->name));

                                train_position_info.next_sensor = get_next_sensor(last_sensor_track_node);
                                update_terminal_train_slot_next_location(train_number, train_slot, (train_position_info.next_sensor == NULL) ? -1 
                                    : sensor_to_id((char*)(train_position_info.next_sensor->name)));

                                if(train_position_info.next_sensor != NULL) {
                                    //Set the predicted nodes for error cases
                                    train_position_info.sensor_error_next_sensor = get_next_sensor(train_position_info.next_sensor);
                                    train_position_info.switch_error_next_sensor = get_next_sensor_switch_broken(last_sensor_track_node);

                                    //NOTE: if you put this back, make sure the sensors aren't null!
                                    //send_term_error_msg("If sensor broken: %s  If switch broken: %s", train_position_info.sensor_error_next_sensor->name, train_position_info.switch_error_next_sensor->name);
                                } else {
                                    train_position_info.sensor_error_next_sensor = NULL;
                                    train_position_info.switch_error_next_sensor = NULL;
                                }

                                finding_initial_position = false;
                                break;
                            }
                        }
                    }
                }

                break;
            case TRAIN_SERVER_SWITCH_CHANGED :
                //Invalidate any predictions we made
                //Kill our conductor
                //Resurrect him with a new delay
                break;
            case TRAIN_SERVER_REGISTER_STOP_SENSOR:
                handle_register_stop_sensor(stop_sensors, ((train_server_msg_t*)message)->num1);
                break;
            case TRAIN_SERVER_FIND_INIT_POSITION:
                finding_initial_position = true;
                send_term_error_msg("Finding train: %d", train_number);
                train_set_speed(train_number, 2);
                break;
            default:
                //Invalid command
                bwprintf(COM2, "Invalid train command: %d from TID: %d\r\n", ((train_server_msg_t*)message)->command, requester);
                ASSERT(0);
                break;
        }
	}
}


void train_position_info_init(train_position_info_t* tpi) {
    tpi->dist_from_last_sensor = 0;
    tpi->ticks_at_last_sensor = 0;
    tpi->last_sensor = NULL;
    tpi->next_sensor_estimated_time = 0;
    tpi->dist_to_next_sensor = 0;
    tpi->average_velocity = 0;

    int i, j;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < 80; ++j) {
            tpi->average_velocities[i][j].average_velocity = 0;
            tpi->average_velocities[i][j].average_velocity_count = 0;
        }
    }

}

void train_server_specialize(tid_t tid, uint32_t train_num, int8_t slot) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_INIT;
    msg.num1 = train_num;
    msg.num2 = slot;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
}


void train_trigger_stop_on_sensor(tid_t tid, int8_t sensor_num) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_REGISTER_STOP_SENSOR;
    msg.num1 = sensor_num;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void train_find_initial_position(tid_t tid) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_FIND_INIT_POSITION;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}


void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, int8_t* stop_sensors,track_node** last_sensor_track_node,train_position_info_t* train_position_info) {
    if(*last_sensor_track_node != NULL) {
        track_node* next_sensor = get_next_sensor(*last_sensor_track_node);//train_position_info->next_sensor;
        track_node* sensor_error_next_sensor = train_position_info->sensor_error_next_sensor;
        track_node* switch_error_next_sensor = train_position_info->switch_error_next_sensor;

        int i;
        uint32_t distance;
        volatile uint32_t* time;
        
        if(next_sensor == NULL) {
            return;
        }
     
        uint32_t expected_group = next_sensor->num / 8;
        uint32_t expected_index = next_sensor->num % 8;
        uint32_t sensor_error_group = 0, sensor_error_index = 0;
        uint32_t switch_error_group = 0, switch_error_index = 0;

        if(sensor_error_next_sensor != NULL) {
            sensor_error_group = sensor_error_next_sensor->num / 8;
            sensor_error_index = sensor_error_next_sensor->num % 8;
        }

        if(switch_error_next_sensor != NULL) {
            switch_error_group = switch_error_next_sensor->num / 8;
            switch_error_index = switch_error_next_sensor->num % 8;
        }
        
        for(i = 0; i < 10; ++i) {
            if((sensor_data[i] & stop_sensors[i]) != 0 ) {
                    //we have have hit our stop sensor
                    train_set_speed(train, 0);   
            }

            //The condition for hitting the sensor that we are expecting next
            if((expected_group == i && (sensor_data[expected_group] & (1 << (7-expected_index))) != 0) ||
                (sensor_error_next_sensor != NULL && sensor_error_group == i && 
                    ((sensor_data[sensor_error_group] & (1 << (7 - sensor_error_index))) != 0)) ||
                (switch_error_next_sensor != NULL && switch_error_group == i && 
                    (sensor_data[switch_error_group] & (1 << (7 - switch_error_index))) != 0)) {
                //Get the timestamp from the sensor data
                time = (volatile uint32_t*)(sensor_data+12);

                if((sensor_error_next_sensor != NULL && sensor_error_group == i && 
                    (sensor_data[sensor_error_group] & (1 << (7 - sensor_error_index))) != 0)) {
                    next_sensor = sensor_error_next_sensor;
                } else if((switch_error_next_sensor != NULL && switch_error_group == i && 
                    (sensor_data[switch_error_group] & (1 << (7 - switch_error_index))) != 0)) {
                    next_sensor = switch_error_next_sensor;
                }

                //Distance between the last sensor and the one we just hit
                distance = distance_between_track_nodes(*last_sensor_track_node, next_sensor);
                    
                int32_t velocity = (distance*100)/(*time -train_position_info->ticks_at_last_sensor);

                uint32_t* average_velocity_count = &train_position_info->average_velocities[(*last_sensor_track_node)->num][next_sensor->num].average_velocity_count; 
                uint32_t* average_velocity = &train_position_info->average_velocities[(*last_sensor_track_node)->num][next_sensor->num].average_velocity;

                *average_velocity = ((train_position_info->average_velocity * (*average_velocity_count)) + velocity) / (*average_velocity_count + 1);
                ++(*average_velocity_count);

                //Send our time in mm / s
                send_term_update_velocity_msg(slot, velocity);
            
                //Currently sends the distance between the last 2 sensors that we just passed by, in mm
                send_term_update_dist_msg(slot, distance );
                int32_t time_difference = *time - train_position_info->next_sensor_estimated_time;

                //Update the error for the train on screen.
                send_term_update_err_msg(slot, (time_difference * ((int32_t)(*average_velocity))) / 100); //Converts to mm distance

                train_position_info->ticks_at_last_sensor = *time;
                //This may come in handy if we need error correction?
                train_position_info->last_sensor = next_sensor;

                //Set our most recent sensor to the sensor we just hit.
                *last_sensor_track_node = next_sensor;
                train_position_info->next_sensor = get_next_sensor(next_sensor);

                //Update the terminal display
                update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)(*last_sensor_track_node)->name));

                if(train_position_info->next_sensor != NULL) {
                    update_terminal_train_slot_next_location(train, slot, sensor_to_id((char*)(train_position_info->next_sensor->name)));

                    train_position_info->dist_to_next_sensor = distance_between_track_nodes(*last_sensor_track_node, train_position_info->next_sensor);

                    train_position_info->average_velocity = train_position_info->average_velocities[(*last_sensor_track_node)->num][next_sensor->num].average_velocity;

                    train_position_info->next_sensor_estimated_time = *time + (train_position_info->dist_to_next_sensor * 100) / train_position_info->average_velocity;
                    //send_term_error_msg("Expected arrival at next sensor: %d Dist: %d   Avg Velocity: %d Velocity: %d", train_position_info->next_sensor_estimated_time, train_position_info->dist_to_next_sensor, train_position_info->average_velocity, velocity);

                    //Update the predicted nodes for error cases
                    train_position_info->sensor_error_next_sensor = get_next_sensor(train_position_info->next_sensor);
                    train_position_info->switch_error_next_sensor = get_next_sensor_switch_broken(*last_sensor_track_node);

                    //NOTE: if you put this back, make sure the sensors aren't null!
                    //send_term_error_msg("If sensor broken: %s  If switch broken: %s", train_position_info->sensor_error_next_sensor->name, train_position_info->switch_error_next_sensor->name);
                } else { 
                    train_position_info->sensor_error_next_sensor = NULL;
                    train_position_info->switch_error_next_sensor = NULL;

                    update_terminal_train_slot_next_location(train, slot, -1);
                }
            }
        }

        int32_t time_to_expected_time = train_position_info->next_sensor_estimated_time - Time();
        send_term_update_dist_msg(slot, (time_to_expected_time * train_position_info->average_velocity) / 100);
    }
}



void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num) {
    int index = sensor_num / 8;
    stop_sensors[index] |= (0x1 << (7 - ((int16_t)sensor_num % 8)));
}


