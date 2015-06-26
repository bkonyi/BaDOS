#include <trains/train_server.h>
#include <trains/track_position_server.h>
#include <trains/train_controller_commander.h>
#include <io/io.h>
#include <terminal/terminal.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))



static void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, int8_t* stop_sensors,track_node** last_sensor_track_node); 
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

    int i;
    for(i = 0; i < 10; ++i) {
        stop_sensors[i] = 0;
    }

    track_node* last_sensor_track_node = NULL;


    //CURRENTLY A STEM CELL TRAIN,
    //need to obtain train info
    Receive(&requester,message, message_size);
    Reply(requester,NULL,0);
    if(((train_server_msg_t*)message)->command != TRAIN_SERVER_INIT) {
        //Our train hasn't been initialized
        ASSERT(0);
    }

    train_number = ((train_server_msg_t*)message)->num1; 
    train_slot   = ((train_server_msg_t*)message)->num2;
    last_sensor_track_node = tps_add_train(train_number);
    KASSERT(last_sensor_track_node != NULL);

    update_terminal_train_slot_current_location(train_number, train_slot, sensor_to_id((char*)last_sensor_track_node->name));
    update_terminal_train_slot_next_location(train_number, train_slot, sensor_to_id((char*)((get_next_sensor(last_sensor_track_node))->name)));

	FOREVER {
		Receive(&requester, message, message_size);
        switch (((train_server_msg_t*)message)->command) {
            case TRAIN_SERVER_SENSOR_DATA:
                //Do calculations for our train.
                handle_sensor_data(train_number, train_slot, ((train_server_sensor_msg_t*)message)->sensors, stop_sensors,&last_sensor_track_node);
                break;
            case TRAIN_SERVER_SWITCH_CHANGED :
                //Invalidate any predictions we made
                //Kill our conductor
                //Resurrect him with a new delay
                break;
            case TRAIN_SERVER_REGISTER_STOP_SENSOR:
                handle_register_stop_sensor(stop_sensors, ((train_server_msg_t*)message)->num1);
                break;
            default:
                //Invalid command
                bwprintf(COM2, "Invalid train command: %d from TID: %d\r\n", ((train_server_msg_t*)message)->command, requester);
                ASSERT(0);
                break;
        }

        Reply(requester, (char*)NULL, 0);
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


void handle_sensor_data(int16_t train, int16_t slot, int8_t* sensor_data, int8_t* stop_sensors,track_node** last_sensor_track_node) {
    track_node* next_sensor = NULL;
    int i;
    uint32_t group,index;
    
    next_sensor = get_next_sensor(*last_sensor_track_node);


    KASSERT(next_sensor != NULL);

    group = next_sensor->num / 8;
    index = next_sensor->num - group * 8;
    
    for(i = 0; i < 10; ++i) {
        if(group == i && (sensor_data[group] & (1 << (7-index))) != 0) {
            //we have now passed our next sensor
            *last_sensor_track_node = next_sensor;

            //Update the terminal display
            update_terminal_train_slot_current_location(train, slot, sensor_to_id((char*)next_sensor->name));
            update_terminal_train_slot_next_location(train, slot, sensor_to_id((char*)((get_next_sensor(next_sensor))->name)));
            

            if((sensor_data[i] & stop_sensors[i]) != 0 ) {
                //we have have hit our stop sensor
                train_set_speed(train, 0);
                
            }
            break;
        }
    }
}



void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num) {
    int index = sensor_num / 8;
    stop_sensors[index] |= (0x1 << (7 - ((int16_t)sensor_num % 8)));
}


