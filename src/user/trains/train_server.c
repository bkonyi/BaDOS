#include <trains/train_server.h>
#include <trains/track_position_server.h>
#include <io/io.h>

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
        switch (((train_server_msg_t*)message)->command) {
            case TRAIN_SERVER_SENSOR_DATA:
                //Do calculations for our train.
                break;
            case  TRAIN_SERVER_SWITCH_CHANGED :
                //Invalidate any predictions we made
                //Kill our conductor
                //Resurrect him with a new delay
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
    Send(tid,(char*)&msg,sizeof(train_server_msg_t),NULL,0);
}
