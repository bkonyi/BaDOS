#include <trains/train_server.h>

#define TRAIN_SERVER_MSG_SIZE (sizeof(train_server_msg_t))
#define TRAIN_SERVER_SENSOR_MSG_SIZE (sizeof(train_server_sensor_msg_t))

void train_server(void) {
    //The bigger of the 2 should be the size we use for receive
    int message_size = (TRAIN_SERVER_MSG_SIZE > TRAIN_SERVER_SENSOR_MSG_SIZE?
                        TRAIN_SERVER_MSG_SIZE:
                        TRAIN_SERVER_SENSOR_MSG_SIZE);
	 int requester;
	char message[message_size]; 
    int train_number = -1;
    train_server_cmd_t* message_command = (train_server_cmd_t*)message;
    train_server_msg_t* train_server_msg = (train_server_msg_t*)message;

    //CURRENTLY A STEM CELL TRAIN,
    //need to obtain train info
    Receive(&requester,(char*) &train_server_msg, sizeof(train_server_msg_t));
    Reply(requester,NULL,0);
    if(*message_command != TRAIN_SERVER_INIT) {
        //Our train hasn't been initialized
        ASSERT(0);
    }
    train_number = train_server_msg->num1; 

	FOREVER {
		Receive(&requester,message,sizeof(char)*10);
        switch (*message_command) {
            case TRAIN_SERVER_SENSOR_DATA:
                break;
            case  TRAIN_SERVER_SWITCH_CHANGED :
                break;
            default:
                break;
        }

	}
}
