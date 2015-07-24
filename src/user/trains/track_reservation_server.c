#include <trains/track_reservation_server.h>
#include <common.h>
#include <syscalls.h>
#include <servers.h>
#include <track_data.h>
#include <terminal/terminal_debug_log.h>
#include <common.h>
#include <track_node.h>
//TRACK_RESERVATION_SERVER_ID

void _send_track_res_msg(track_res_msg_type_t type, track_node* node, int train_num, track_res_msg_t* response);
bool _handle_node_reserve(track_node* node, int train_num);
void _handle_node_release(track_node* node, int train_num);
void _handle_reservation_init(void);
void _print_reserved_tracks(reserved_node_queue_t* res_queue,int train_num);
bool _handle_track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance);

track_node* _next_reservation_node(track_node* node, int train_num);

void track_reservation_server(void) {
	track_res_msg_t message;
	int requester;
	char char_message[(sizeof(track_res_msg_t))];
	track_res_msg_t* response = (track_res_msg_t*)char_message;
	bool* bool_response = (bool*)char_message;
	response->type = TR_UNSET;
	int response_size =0;

	//Initialize
	_handle_reservation_init();
	
	FOREVER {
		Receive(&requester,(char*)&message,sizeof(track_res_msg_t));
		switch(message.type){
			case TR_RESERVE:
			
				if(_handle_node_reserve(message.our_node, message.train_num)){
					response->type = TR_RESERVE_APPROVE;
				} else {
					response->type = TR_RESERVE_REJECT;
				}
				response_size = sizeof(track_res_msg_t);
				break;
			case TR_RELEASE:
				_handle_node_release(message.our_node, message.train_num);
				response_size =0;
				break;
			case TR_MAKE_RESERVATIONS:
				ASSERT(message.res_queue != NULL);
				*bool_response = _handle_track_handle_reservations(message.res_queue,message.train_num,message.our_node,message.offset_in_node,message.stopping_distance);
					response_size =sizeof(bool);

				break;
			default:
				ASSERT(0);
		}
		Reply(requester,char_message, response_size);

	}
}

void _handle_reservation_init(void) {
	int requester, i;
	track_res_msg_t message;
	Receive(&requester,(char*)&message,sizeof(track_res_msg_t));
	ASSERT(message.type == TR_INIT);

	track_node* node_base = message.our_node;

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
	track_node_flip(node)->reserved_by = num;
}

void _send_track_res_msg(track_res_msg_type_t type, track_node* node, int train_num, track_res_msg_t* response){
	track_res_msg_t msg;
	int response_size;

	msg.type = type;
	msg.our_node = node;
	msg.train_num = train_num;
	ASSERT(node != NULL);
	ASSERT(msg.our_node != NULL);
	if(response == NULL){
		response_size = 0;
	}else {
		response_size = sizeof(track_res_msg_t);
	}
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&msg,sizeof(track_res_msg_t),(char*)response,response_size);

}
bool _handle_node_reserve(track_node* node, int train_num){
	ASSERT(node!=NULL);
	track_node* flip =track_node_flip(node);
	if(flip->type == NODE_BRANCH){
		node = flip;
	}
	if(node->type == NODE_BRANCH){
		if(node->reserved_by == train_num || node->reserved_by == -1){
			//we need to reserve the whole branch
			track_node *left, *right;
			left = node->edge[0].dest->reverse;
			right = node->edge[1].dest->reverse;
			if(!(left->reserved_by == -1 || left->reserved_by == train_num)){
				send_term_debug_log_msg("fail node %s(%d) was was reserved by %d",left->name,train_num,left->reserved_by);
			}
			ASSERT(right->reserved_by == -1 || right->reserved_by == train_num);
			_set_track_node_reservation(left,train_num);
			_set_track_node_reservation(right,train_num);
			ASSERT(node->reserved_by == train_num && left->reserved_by == train_num && right->reserved_by == train_num);
			//send_term_debug_log_msg("train: %d A RESERVED: %s",train_num,node->name);
			return  true;
		}else{
			//send_term_debug_log_msg("Reserve FAILED was reserved_by %d tried res of %d",node->reserved_by,train_num);
			return false;
		}
	}else {
		if(node->reserved_by == -1  || node->reserved_by == train_num){
			if(flip->reserved_by != -1 && flip->reserved_by != train_num){
				send_term_debug_log_msg("_handle_node_reserve fail %s resby: %d",track_node_flip(node)->name,track_node_flip(node)->reserved_by);
				Delay(200);
				ASSERT(0);
			}
			_set_track_node_reservation(node,train_num);
			//send_term_debug_log_msg("train: %d B RESERVED: %s",train_num,node->name);
			return true;
		}else{
			//send_term_debug_log_msg("Reserve FAILED was reserved_by %d tried res of %d",node->reserved_by,train_num);
			return false;
		}
		
	}
	
}
void _handle_node_release(track_node* node, int train_num){
	ASSERT(node!=NULL);
	track_node* flip =track_node_flip(node);
	if(flip->type == NODE_BRANCH){
		node = flip;
	}
	if(node->type == NODE_BRANCH) {
		ASSERT(node->reserved_by == -1 || node->reserved_by == train_num);
		track_node *left, *right;
		left = node->edge[0].dest->reverse;
		right = node->edge[1].dest->reverse;

		_set_track_node_reservation(left,-1);
		_set_track_node_reservation(right,-1);
		ASSERT(node->reserved_by == -1 && left->reserved_by == -1 && right->reserved_by == -1);
		//send_term_debug_log_msg("train: %d A released: %s",train_num,node->name);
	}else{
		if(!(node->reserved_by == train_num || node->reserved_by == -1)) {
			send_term_debug_log_msg("Train %d tried to release %s res by: %d",train_num,node->name,node->reserved_by);
			ASSERT(0);
		}
		//ASSERT(node->reverse->reserved_by != -1);
		_set_track_node_reservation(node,-1);
		//send_term_debug_log_msg("train: %d B released: %s",train_num,node->name);
	}
	
}
track_node* _next_reservation_node(track_node* node, int train_num) {
	if(node == NULL) return NULL;

	track_node *left, *right;
	left = node->edge[0].dest;
	right = node->edge[1].dest;
	if(left->reserved_by == train_num) {
		return left;
	}else if(right->reserved_by == train_num){
		return right;
	}else{ 
		//No adjacent nodes return NULL
		return NULL;
	}
}

bool _reserve_tracks_from_point(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node,int stopping_distance) {
	bool nodes_added = false;
	ASSERT(our_node !=NULL);
	ASSERT(res_queue != NULL);
	if(our_node == NULL) return false;
	track_node* iterator_node;
	//send_term_debug_log_msg("Reserve %d from %s with off %d",stopping_distance, our_node->name,offset_in_node);
	//TODO: TRAIN_PANIC
	if(!(our_node->reserved_by == train_num )){
		send_term_debug_log_msg("%s(%d) Was reserved by: %d",our_node->name,train_num,our_node->reserved_by);
		Delay(2000);
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
		
		if(!_handle_node_reserve(iterator_node,train_num)) {
			return false;
		}else{
            bool exists_in;
            RESERVED_VALUE_EXISTS_IN(*res_queue,iterator_node,exists_in);
            if(!exists_in){

            	nodes_added = true;
                RESERVED_PUSH_BACK(*res_queue,iterator_node);
            }
        }

		stopping_distance-= get_track_node_length(iterator_node);
	}
	if(nodes_added){
		//_print_reserved_tracks(res_queue,train_num);
		//send_term_debug_log_msg("PRONT");
		//track_clear_reservations(res_queue, train_num, our_node, offset_in_node,stopping_distance);
		_print_reserved_tracks(res_queue,train_num);
	}
	return true;
}
void _release_track_from_point_behind(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {
	bool nodes_removed = false;
	int dist =  -1 * (offset_in_node );
	track_node* iterator_node;
	// /send_term_debug_log_msg("release(%d) ournode %s off: %d",train_num,our_node->name,offset_in_node);
	/*
	if(iterator_node->type == NODE_BRANCH || iterator_node->type == NODE_MERGE) {
		dist+=get_track_node_length(iterator_node);
		iterator_node = _next_reservation_node(iterator_node, train_num);
	}*/
		int count =0 ;
		bool found_1_sens_behind = false;
	for(iterator_node = track_node_flip(our_node); iterator_node != NULL && iterator_node->reserved_by == train_num; iterator_node= _next_reservation_node (iterator_node, train_num)){
		
		
		if(found_1_sens_behind || dist>0){
			//Keep iterating until we've hit enough track to compensate for the length of the train
			_handle_node_release(iterator_node,train_num);
			RESERVED_REMOVE_VALUE(*res_queue,iterator_node);
			nodes_removed = true;
		}else if(count>=1 && iterator_node->type == NODE_SENSOR){
			found_1_sens_behind = true;
		}
		
		count++;
		dist+=get_track_node_length(iterator_node);
	}
	if(nodes_removed == true) {
		_print_reserved_tracks(res_queue,train_num);
	}

}
void _release_track_from_point_ahead(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {
	bool nodes_removed = false;
	int dist =  -1 * (offset_in_node +50);
	track_node* iterator_node = our_node;
	

	if(iterator_node->type == NODE_BRANCH || iterator_node->type == NODE_MERGE) {
		iterator_node = _next_reservation_node(iterator_node, train_num);
		dist+=get_track_node_length(iterator_node);
	}
	for(; iterator_node != NULL && iterator_node->reserved_by == train_num; iterator_node= _next_reservation_node (iterator_node, train_num)){
		
		if(dist >0  ){
			//Keep iterating until we've hit enough track to compensate for the length of the train
			track_release_node(iterator_node,train_num);
			RESERVED_REMOVE_VALUE(*res_queue,iterator_node);
			nodes_removed = true;
		}
		dist+=get_track_node_length(iterator_node);
	}
	if(nodes_removed == true) {
		_print_reserved_tracks(res_queue,train_num);
	}

}
bool _handle_track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance){
	bool bool_result =_reserve_tracks_from_point(res_queue,train_num, our_node, offset_in_node, stopping_distance);
	_release_track_from_point_behind(res_queue, train_num, our_node, offset_in_node, stopping_distance);
	return bool_result;
}
bool track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {
	ASSERT(res_queue != NULL);
	ASSERT(our_node != NULL);
	track_res_msg_t message;

	message.type = TR_MAKE_RESERVATIONS;
	message.res_queue = res_queue;
	message.train_num = train_num;
	message.our_node = our_node;
	message.offset_in_node = offset_in_node;
	message.stopping_distance = stopping_distance;

	bool bool_result;
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&message, sizeof(track_res_msg_t),(char*)&bool_result,sizeof(bool));
	return bool_result;

	
}
void track_clear_reservations(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance) {
	_release_track_from_point_ahead(res_queue, train_num, our_node, offset_in_node, stopping_distance);
	_release_track_from_point_behind(res_queue, train_num, our_node, offset_in_node, stopping_distance);
}


void _print_reserved_tracks(reserved_node_queue_t* res_queue,int train_num){
	track_node* iterator_node = res_queue->head;
	char buff[DEBUG_LOG_MAX_LEN];
	char* buff_it = buff;
	sprintf(buff_it,"RES TR %d: ",train_num);
	buff_it += strlen(buff_it);
	while (iterator_node != NULL) {
		sprintf(buff_it,"%s ",iterator_node->name);
		buff_it += strlen(buff_it);
		iterator_node = iterator_node->next_reserved;
	}

	send_term_debug_log_msg("%s",buff);
}

bool track_compare_reserved_node(track_node* node, track_node* b){
	track_node* flip =track_node_flip(node);
	if(flip->type == NODE_BRANCH){
		node = flip;
	}
	if(node->type == NODE_BRANCH){
		track_node *left, *right;
		left = node->edge[0].dest->reverse;
		right = node->edge[1].dest->reverse;
		if(left == b || right == b || node == b){
			return true;
		}
	}else{
		if( track_node_flip(node) == b || node  == b ) {
			return true;
		}
	}
	return false;
	
}

