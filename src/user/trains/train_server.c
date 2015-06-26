#include <trains/train_server.h>
#include <trains/track_position_server.h>
#include <trains/train_controller_commander.h>
#include <io/io.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))

static void handle_sensor_data(int train, int8_t* sensor_data, int8_t* stop_sensors); 
static void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num);

void train_server(void) {
    //The bigger of the 2 should be the size we use for receive
    int message_size = (TRAIN_SERVER_MSG_SIZE > TRAIN_SERVER_SENSOR_MSG_SIZE?
                        TRAIN_SERVER_MSG_SIZE:
                        TRAIN_SERVER_SENSOR_MSG_SIZE);
	int requester;
	char message[message_size]; 
    int train_number = -1;
    int8_t stop_sensors[10];

    int i;
    for(i = 0; i < 10; ++i) {
        stop_sensors[i] = 0;
    }

    //CURRENTLY A STEM CELL TRAIN,
    //need to obtain train info
    Receive(&requester,message, message_size);
    Reply(requester,NULL,0);
    if(((train_server_msg_t*)message)->command != TRAIN_SERVER_INIT) {
        //Our train hasn't been initialized
        bwprintf(COM2,"cmd : %d \r\n", ((train_server_msg_t*)message)->command);
        ASSERT(0);
    }

    train_number = ((train_server_msg_t*)message)->num1; 
    tps_add_train(train_number);

	FOREVER {
		Receive(&requester, message, message_size);
        int i;
        switch (((train_server_msg_t*)message)->command) {
            case TRAIN_SERVER_SENSOR_DATA:

                
                //printf(COM2, "\r\n\r\n");
                for(i = 0; i < 10; ++i) {
                    //printf(COM2,"FIRST Stop Bits[%d]: 0x%x\r\n", i, stop_sensors[i]);
                }
                            //Do calculations for our train.
                handle_sensor_data(train_number, ((train_server_sensor_msg_t*)message)->sensors, stop_sensors);
                break;
            case TRAIN_SERVER_SWITCH_CHANGED :
                //Invalidate any predictions we made
                //Kill our conductor
                //Resurrect him with a new delay
                break;
            case TRAIN_SERVER_REGISTER_STOP_SENSOR:
                handle_register_stop_sensor(stop_sensors, ((train_server_msg_t*)message)->num1);
                /*printf(COM2, "\r\nSTOP SENSOR TRAIN: %d SENSOR: %d\r\n", train_number, ((train_server_msg_t*)message)->num1);

                int i;
                for(i = 0; i < 10; ++i) {
                    printf(COM2, "\r\nStop[%d]: 0x%x\r\n", i, stop_sensors[i]);
                }*/

                //getc(COM2);
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


void train_server_specialize(tid_t tid, uint32_t train_num) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_INIT;
    msg.num1 = train_num;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), NULL, 0);
}

void train_trigger_stop_on_sensor(tid_t tid, int8_t sensor_num) {
    train_server_msg_t msg;
    msg.command = TRAIN_SERVER_REGISTER_STOP_SENSOR;
    msg.num1 = sensor_num;
    Send(tid, (char*)&msg, sizeof(train_server_msg_t), (char*)NULL, 0);
}

void handle_sensor_data(int train, int8_t* sensor_data, int8_t* stop_sensors) {
    int i;
    //printf(COM2, "\r\n\r\n");
    for(i = 0; i < 10; ++i) {
        //printf(COM2,"Stop Bits[%d]: 0x%x\r\n", i, stop_sensors[i]);
        if(sensor_data[i] & stop_sensors[i]) {
            train_set_speed(train, 0);
            break;
        }
    }
}

void handle_register_stop_sensor(int8_t* stop_sensors, int8_t sensor_num) {
    int index = sensor_num / 8;
    stop_sensors[index] |= (0x1 << (7 - ((int16_t)sensor_num % 8)));
}

