#include <trains/track_reservation_server.h>
#include <common.h>
#include <syscalls.h>
#include <servers.h>
#include <track_data.h>
#include <terminal/terminal_debug_log.h>

//TRACK_RESERVATION_SERVER_ID

void _send_track_res_msg(track_res_msg_type_t type, track_node* node, int train_num, track_res_msg_t* response);
bool _handle_node_reserve(track_node* node, int train_num);
void _handle_node_release(track_node* node, int train_num);
void _handle_reservation_init(void);

void track_reservation_server(void) {
	track_res_msg_t message;
	int requester;
	track_res_msg_t response;
	response.type = TR_UNSET;
	
	//Initialize
	_handle_reservation_init();
	
	FOREVER {
		Receive(&requester,(char*)&message,sizeof(track_res_msg_t));
		switch(message.type){
			case TR_RESERVE:
				if(_handle_node_reserve(message.node, message.train_num)){
					response.type = TR_RESERVE_APPROVE;
				} else {
					response.type = TR_RESERVE_REJECT;
				}
				break;
			case TR_RELEASE:
				_handle_node_release(message.node, message.train_num);
				break;
			default:
				ASSERT(0);
		}
		Reply(requester,(char*)&response, sizeof(track_res_msg_t));

	}
}

void _handle_reservation_init(void) {
	int requester, i;
	track_res_msg_t message;
	Receive(&requester,(char*)&message,sizeof(track_res_msg_t));
	ASSERT(message.type == TR_INIT);
	track_node* node_base = message.node;

	for(i = 0; i < TRACK_MAX; i++) {
		node_base[0].reserved_by = -1;
	}

	Reply(requester,NULL,0);
	send_term_debug_log_msg("Reservation Server Initialized");
}

//Expects the base node for the track_nodes so all of them can be initialized
void track_reservation_init(track_node* base_node){
	send_term_debug_log_msg("Callint track_re_init");
	_send_track_res_msg(TR_INIT,base_node,0,NULL);
}

bool track_reserve_node(track_node* node,int train_num) {
	track_res_msg_t response;
	_send_track_res_msg(TR_RESERVE,node,train_num,&response);
	if(response.type ==TR_RESERVE_APPROVE){
		return true;
	}else if(response.type == TR_RESERVE_REJECT){
		return false;
	}else{
		ASSERT(0);
		return false;	
	}

}

void track_release_node(track_node* node,int train_num) {
	_send_track_res_msg(TR_RELEASE,node,train_num,NULL);
}

void _send_track_res_msg(track_res_msg_type_t type, track_node* node, int train_num, track_res_msg_t* response){
	track_res_msg_t msg;
	int response_size;

	msg.type = type;
	msg.node = node;
	msg.train_num = train_num;

	if(response == NULL){
		response_size = 0;
	}else {
		response_size = sizeof(track_res_msg_t);
	}
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&msg,sizeof(track_res_msg_t),(char*)response,response_size);

}
bool _handle_node_reserve(track_node* node, int train_num){
	if(node->reserved_by == -1){
		node->reserved_by = train_num;
		return true;
	}
	return false;
}
void _handle_node_release(track_node* node, int train_num){
	ASSERT(node->reserved_by == train_num);
	node->reserved_by = -1;
}
