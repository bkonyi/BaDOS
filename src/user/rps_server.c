#include "rps_server.h"
#include "syscalls.h"

void rps_server_task(void){
	RegisterAs("RPSSERVER");
	int sender_tid;
	rps_msg msg;
	Receive(&sender_tid,(char*)&msg, sizeof(rps_msg));
	switch(msg.type){
		case RPS_SIGNUP:
			//TODO
			break;
		case RPS_PLAY:
			//TODO
			break;
		case RPS_QUIT:
			//TODO
			break;
	}
}
