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
track_node* _next_reservation_node(track_node* node, int train_num);

void track_reservation_server(void) {
	track_res_msg_t message;
	int requester;
	track_res_msg_t response;
	response.type = TR_UNSET;
	int response_size =0;

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
				response_size = sizeof(track_res_msg_t);
				break;
			case TR_RELEASE:
				_handle_node_release(message.node, message.train_num);
				response_size =0;
				break;
			default:
				ASSERT(0);
		}
		Reply(requester,(char*)&response, response_size);

	}
}

void _handle_reservation_init(void) {
	int requester, i;
	track_res_msg_t message;
	Receive(&requester,(char*)&message,sizeof(track_res_msg_t));
	ASSERT(message.type == TR_INIT);

	track_node* node_base = message.node;

	for(i = 0; i < TRACK_MAX; i++) {
		node_base[i].reserved_by = -1;
		if(node_base[i].index != i){
			send_term_debug_log_msg("Trying to set index %d on index %d",i,node_base[i].index);
			Delay(500);
			ASSERT(0);
		}
	}

	Reply(requester,NULL,0);
	send_term_debug_log_msg("Reservation Server Initialized");
}

//Expects the base node for the track_nodes so all of them can be initialized
void track_reservation_init(track_node* base_node){
	send_term_debug_log_msg("Callint track_re_init 0x%x",(uint32_t)base_node);
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
void _set_track_node_reservation(track_node* node, int num){
	ASSERT(node != NULL);
	node->reserved_by = num;
	get_next_track_node(node)->reverse->reserved_by = num;
}

void _send_track_res_msg(track_res_msg_type_t type, track_node* node, int train_num, track_res_msg_t* response){
	track_res_msg_t msg;
	int response_size;

	msg.type = type;
	msg.node = node;
	msg.train_num = train_num;
	ASSERT(node != NULL);
	ASSERT(msg.node != NULL);
	if(response == NULL){
		response_size = 0;
	}else {
		response_size = sizeof(track_res_msg_t);
	}
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&msg,sizeof(track_res_msg_t),(char*)response,response_size);

}
bool _handle_node_reserve(track_node* node, int train_num){
	ASSERT(node!=NULL);
	if(node->reserved_by == -1 ){
		ASSERT(get_next_track_node( node)->reverse->reserved_by == -1);
		_set_track_node_reservation(node,train_num);
		send_term_debug_log_msg("train: %d RESERVED: %s",train_num,node->name);
		return true;
	}else if(node->reserved_by == train_num){
		return true;
	}
	send_term_debug_log_msg("Reserve FAILED was reserved_by %d tried res of %d",node->reserved_by,train_num);
	return false;
}
void _handle_node_release(track_node* node, int train_num){
	ASSERT(node!=NULL);
	if(!(node->reserved_by == train_num || node->reserved_by == -1)) {
		send_term_debug_log_msg("Train %d tried to release %s res by: %d",train_num,node->name,node->reserved_by);
		ASSERT(0);
	}
	//ASSERT(node->reverse->reserved_by != -1);
	_set_track_node_reservation(node,-1);
	send_term_debug_log_msg("train: %d released: %s",train_num,node->name);
}
track_node* _next_reservation_node(track_node* node, int train_num) {
	ASSERT(node->reserved_by == train_num);
	if(node == NULL) return NULL;

	if(node->edge[DIR_AHEAD].dest->reserved_by == train_num) {
		return node->edge[DIR_AHEAD].dest;
	}else if(node->edge[DIR_CURVED].dest->reserved_by == train_num){
		return node->edge[DIR_CURVED].dest;
	}else{ 
		//No adjacent nodes return NULL
		return NULL;
	}
}

bool _reserve_tracks_from_point(int train_num, track_node* our_node, int offset_in_node,int stopping_distance) {
	ASSERT(our_node !=NULL);
	if(our_node == NULL) return false;
	track_node* iterator_node;
	//send_term_debug_log_msg("Reserve %d from %s with off %d",stopping_distance, our_node->name,offset_in_node);
	//TODO: TRAIN_PANIC
	if(!(our_node->reserved_by == train_num )){
		send_term_debug_log_msg("%s Was reserved by: %d",our_node->name,our_node->reserved_by);
		Delay(200);
		ASSERT(0);
	}

	//we add our offset into our current node so that when we subtract it in the
		//loop that follows then then it takes into account that the have that
		//much distance that won't contribute to the stopping distance
	stopping_distance+= offset_in_node;
	stopping_distance += 500; // TODO: When the trains actually get calibrated remove this
	for(iterator_node = our_node; iterator_node != NULL && stopping_distance >= 0;iterator_node= get_next_track_node (iterator_node)){
		//send_term_debug_log_msg("track try to reserve %s",iterator_node->name);
/*		if(iterator_node->reserved_by != train_num && iterator_node->reserved_by != -1){
			//TODO: Train panic, and say we couldn't reserve it ?? Maybe?
			return false;
		}*/
		
		if(!track_reserve_node(iterator_node,train_num)) {
			return false;
		}

		stopping_distance-= get_track_node_length(iterator_node);
	}
	return true;
}
void _release_track_from_point(int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {

	int dist =  -1 * (offset_in_node - 200);
	track_node* iterator_node;
	
	for(iterator_node = our_node->reverse; iterator_node != NULL && iterator_node->reserved_by == train_num; iterator_node= get_next_track_node (iterator_node)){
		
		if(dist >0  ){
			//Keep iterating until we've hit enough track to compensate for the length of the train
			//send_term_debug_log_msg("tp %s off %d",our_node->name,offset_in_node);
			track_release_node(iterator_node,train_num);
		}
		dist+=get_track_node_length(iterator_node);
	}

}
bool track_handle_reservations(int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {
	bool bool_result =_reserve_tracks_from_point(train_num, our_node, offset_in_node, stopping_distance);
	_release_track_from_point(train_num, our_node, offset_in_node, stopping_distance);
	return bool_result;
}

