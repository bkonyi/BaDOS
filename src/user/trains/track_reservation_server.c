#include <trains/track_reservation_server.h>
#include <common.h>
#include <syscalls.h>
#include <servers.h>
#include <track_data.h>
#include <terminal/terminal_debug_log.h>
#include <common.h>
#include <track_node.h>
//TRACK_RESERVATION_SERVER_ID

#define TRACK_RESERVATIONS_ENABLED

void _send_track_res_msg(track_res_msg_type_t type, track_node* node,reserved_node_queue_t* res_queue, int train_num, track_res_msg_t* response);
bool _handle_node_reserve(reserved_node_queue_t* res_queue,track_node* node, int train_num);
void _handle_node_release(reserved_node_queue_t* res_queue,track_node* node, int train_num);
void _handle_reservation_init(void);
void _print_reserved_tracks(reserved_node_queue_t* res_queue,int train_num);
bool _handle_track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance, bool initial_reservation);
bool _has_adjacent_reserved(track_node* node, int train_num);
bool _check_reserve_path_from_point_with_switch(reserved_node_queue_t* res_queue, int train_num, track_node* our_node,int32_t offset_in_node,int stopping_distance, int switch_num);

track_node* _next_reservation_node(track_node* node, int train_num);
static bool _handle_recursive_release_nodes(reserved_node_queue_t* res_queue,track_node* our_node, int train_num);

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
			
				if(_handle_node_reserve(message.res_queue,message.our_node, message.train_num)){
					response->type = TR_RESERVE_APPROVE;
				} else {
					response->type = TR_RESERVE_REJECT;
				}
				response_size = sizeof(track_res_msg_t);
				break;
			case TR_RELEASE:
				_handle_node_release(message.res_queue,message.our_node, message.train_num);
				response_size =0;
				break;
			case TR_MAKE_RESERVATIONS:
				ASSERT(message.res_queue != NULL);
				*bool_response = _handle_track_handle_reservations(message.res_queue,message.train_num,message.front_data,message.back_data,message.stopping_distance,message.initial_reservation);
					response_size =sizeof(bool);

				break;
			case TR_CHECK_RESERVATIONS_WITH_SWITCH:
				*bool_response = _check_reserve_path_from_point_with_switch(message.res_queue,message.train_num,message.front_data->node,message.front_data->offset,message.stopping_distance,message.switch_num);	
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
			//send_term_debug_log_msg("Trying to set index %d on index %d",i,node_base[i].index);
			Delay(500);
			ASSERT(0);
		}
	}

	Reply(requester,NULL,0);
	//send_term_debug_log_msg("Reservation Server Initialized");
}

//Expects the base node for the track_nodes so all of them can be initialized
void track_reservation_init(track_node* base_node){
	//send_term_debug_log_msg("Callint track_re_init 0x%x",(uint32_t)base_node);
	_send_track_res_msg(TR_INIT,base_node,NULL,0,NULL);
}

bool track_reserve_node(reserved_node_queue_t* res_queue, track_node* node,int train_num) {
#ifdef TRACK_RESERVATIONS_ENABLED
	track_res_msg_t response;
	_send_track_res_msg(TR_RESERVE,node,res_queue,train_num,&response);
	if(response.type ==TR_RESERVE_APPROVE){
		return true;
	}else if(response.type == TR_RESERVE_REJECT){
		return false;
	}else{
		ASSERT(0);
		return false;	
	}
#else
	return true;
#endif
}
bool _has_adjacent_reserved(track_node* node, int train_num) {
	track_node* flip = track_node_flip(node);

	//Also redundant but I want to be sure

	if(node->reserved_by == train_num) return true;
	if(node->reverse->reserved_by  == train_num) return true;
	if(node->edge[DIR_AHEAD].dest->reserved_by == train_num) return true;
	if(node->type == NODE_BRANCH && node->edge[DIR_CURVED].dest->reserved_by == train_num) return true;

	if(flip->reserved_by == train_num) return true;
	if(flip->reverse->reserved_by  == train_num) return true;
	if(flip->edge[DIR_AHEAD].dest->reserved_by == train_num) return true;
	if(flip->type == NODE_BRANCH && flip->edge[DIR_CURVED].dest->reserved_by == train_num) return true;

	return false;
}

void track_release_node(track_node* node,int train_num) {
#ifdef TRACK_RESERVATIONS_ENABLED
	_send_track_res_msg(TR_RELEASE,node,NULL,train_num,NULL);
#endif	
}
void _set_track_node_reservation(track_node* node, int num){
	ASSERT(node != NULL);
	track_node* flip  = track_node_flip(node);

	//I know this is redundant but I don't want to leave that gap anymore :P
	node->reserved_by = num;
	node->edge[DIR_AHEAD].reverse->src->reserved_by = num;
	if(node->type == NODE_BRANCH){
		node->edge[DIR_CURVED].reverse->src->reserved_by = num;
	}

	flip->reserved_by = num;
	flip->edge[DIR_AHEAD].reverse->src->reserved_by = num;
	if(flip->type == NODE_BRANCH){
		flip->edge[DIR_CURVED].reverse->src->reserved_by = num;
	}

}

void _send_track_res_msg(track_res_msg_type_t type, track_node* node,reserved_node_queue_t* res_queue, int train_num, track_res_msg_t* response){
	track_res_msg_t msg;
	int response_size;

	msg.type = type;
	msg.our_node = node;
	msg.train_num = train_num;
	msg.res_queue = res_queue;
	ASSERT(node != NULL);
	ASSERT(msg.our_node != NULL);
	if(response == NULL){
		response_size = 0;
	}else {
		response_size = sizeof(track_res_msg_t);
	}
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&msg,sizeof(track_res_msg_t),(char*)response,response_size);

}
bool _handle_node_reserve(reserved_node_queue_t* res_queue,track_node* node, int train_num){
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
			RESERVED_PUSH_BACK(*res_queue,left);
			_set_track_node_reservation(right,train_num);
			RESERVED_PUSH_BACK(*res_queue,right);


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
			RESERVED_PUSH_BACK(*res_queue,node);
			//send_term_debug_log_msg("train: %d B RESERVED: %s",train_num,node->name);
			return true;
		}else{
			//send_term_debug_log_msg("Reserve FAILED was reserved_by %d tried res of %d",node->reserved_by,train_num);
			return false;
		}
		
	}
	
}
void _handle_node_release(reserved_node_queue_t* res_queue,track_node* node, int train_num){
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
		RESERVED_REMOVE_VALUE(*res_queue,left);
		_set_track_node_reservation(right,-1);
		RESERVED_REMOVE_VALUE(*res_queue,right);

		ASSERT(node->reserved_by == -1 && left->reserved_by == -1 && right->reserved_by == -1);
		//send_term_debug_log_msg("train: %d A released: %s",train_num,node->name);
	}else{
		if(!(node->reserved_by == train_num || node->reserved_by == -1)) {
			send_term_debug_log_msg("Train %d tried to release %s res by: %d",train_num,node->name,node->reserved_by);
			ASSERT(0);
		}
		//ASSERT(node->reverse->reserved_by != -1);
		_set_track_node_reservation(node,-1);
		RESERVED_REMOVE_VALUE(*res_queue,node);
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

bool _reserve_tracks_from_point(reserved_node_queue_t* res_queue, int train_num, track_node* our_node,int32_t offset_in_node,int stopping_distance, bool initial_reservation) {

	bool nodes_added = false;
	ASSERT(our_node !=NULL);
	ASSERT(res_queue != NULL);
	if(our_node == NULL) return false;
	track_node* iterator_node;
	//send_term_debug_log_msg("Reserve %d from %s with off %d",stopping_distance, our_node->name,offset_in_node);
	//TODO: TRAIN_PANIC
	if( !initial_reservation && !(our_node->reserved_by == train_num )){
		if(our_node->reserved_by == -1 && _has_adjacent_reserved(our_node,train_num)){
			_reserve_tracks_from_point(res_queue,train_num,our_node,offset_in_node, 200,true);

		}else if(!initial_reservation && our_node->reserved_by != train_num && our_node->reserved_by != -1 && _has_adjacent_reserved(our_node,train_num)){
			return false;
		}else{
			send_term_debug_log_msg("%s(%d) Was reserved by: %d",our_node->name,train_num,our_node->reserved_by);
			Delay(20);
			ASSERT(0);
		}
		
	}

	//we add our offset into our current node so that when we subtract it in the
		//loop that follows then then it takes into account that the have that
		//much distance that won't contribute to the stopping distance
	stopping_distance+= offset_in_node;

	stopping_distance += 210; // TODO: When the trains actually get calibrated remove this

	for(iterator_node = our_node; iterator_node != NULL && stopping_distance >= 0;iterator_node= get_next_track_node (iterator_node)){
		//send_term_debug_log_msg("track try to reserve %s",iterator_node->name);
/*		if(iterator_node->reserved_by != train_num && iterator_node->reserved_by != -1){
			//TODO: Train panic, and say we couldn't reserve it ?? Maybe?
			return false;
		}*/
		
		if(!_handle_node_reserve(res_queue, iterator_node,train_num)) {
			return false;
		}else{

            bool exists_in;
            RESERVED_VALUE_EXISTS_IN(*res_queue,iterator_node,exists_in);
            if(!exists_in){
            	nodes_added = true;
                //RESERVED_PUSH_BACK(*res_queue,iterator_node);
            }
        }

		stopping_distance-= get_track_node_length(iterator_node);
	}
	if(nodes_added){
		//_print_reserved_tracks(res_queue,train_num);
		//send_term_debug_log_msg("PRONT");
		//track_clear_reservations(res_queue, train_num, our_node, offset_in_node,stopping_distance);
		//send_term_debug_log_msg("TRRES %s sd %d",our_node->name,offset_in_node);
		_print_reserved_tracks(res_queue,train_num);
	}
	return true;
}


bool _check_reserve_path_from_point_with_switch(reserved_node_queue_t* res_queue, int train_num, track_node* our_node,int32_t offset_in_node,int stopping_distance, int switch_num) {

	ASSERT(is_valid_switch_number(switch_num));
	bool nodes_added = false;
	ASSERT(our_node !=NULL);
	ASSERT(res_queue != NULL);

	if(our_node == NULL) return false;
	track_node* iterator_node;
	

	//we add our offset into our current node so that when we subtract it in the
		//loop that follows then then it takes into account that the have that
		//much distance that won't contribute to the stopping distance
	stopping_distance+= offset_in_node;

	stopping_distance += 210; // TODO: When the trains actually get calibrated remove this

	for(iterator_node = our_node; 
		iterator_node != NULL && stopping_distance >= 0; 
		iterator_node=((iterator_node->type == NODE_BRANCH && iterator_node->num == switch_num)
		 		?  get_next_track_node_assuming_switch (iterator_node)
		 		:  get_next_track_node (iterator_node)) ) {
		//send_term_debug_log_msg("track try to reserve %s",iterator_node->name);
/*		if(iterator_node->reserved_by != train_num && iterator_node->reserved_by != -1){
			//TODO: Train panic, and say we couldn't reserve it ?? Maybe?
			return false;
		}*/
		
		//Pretend like we are going to reserve the track node, then after we find out whether we could, unreserve it
		if(!_handle_node_reserve(res_queue, iterator_node,train_num)) {
			_handle_node_reserve(res_queue, iterator_node,-1);
			return false;
		}else{

            bool exists_in;
            RESERVED_VALUE_EXISTS_IN(*res_queue,iterator_node,exists_in);
            if(!exists_in){
            	nodes_added = true;
            }
        }
        _handle_node_reserve(res_queue, iterator_node,-1);

		stopping_distance -= ((iterator_node->type == NODE_BRANCH && iterator_node->num == switch_num)
		 		?  get_track_node_length_assuming_switch (iterator_node)
		 		:  get_track_node_length(iterator_node));
	}
	
	return true;
}

void _release_track_from_point_behind(reserved_node_queue_t* res_queue, int train_num, track_node* our_node,int32_t offset_in_node, int stopping_distance) {


	track_node_data_t tip_location = track_get_node_location(our_node,offset_in_node-200);
	 track_flip_node_data(&tip_location);
	 track_node* first_node_to_remove = get_next_track_node(tip_location.node);
	 if(get_next_track_node !=NULL) first_node_to_remove = get_next_track_node(first_node_to_remove);

	 track_touch_node(tip_location.node,true);
	 //send_term_debug_log_msg("Starting recursion tr %d", train_num);
	 if(_handle_recursive_release_nodes(res_queue, first_node_to_remove,train_num)){
	 	//send_term_debug_log_msg("TRRELL removing from (%s || %s )",first_node_to_remove->name,track_node_flip(first_node_to_remove)->name);
 		_print_reserved_tracks(res_queue,train_num);
	 }
	// send_term_debug_log_msg("Done Starting recursion tr %d", train_num);
	 track_touch_node(tip_location.node,false);

}

bool _handle_track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance, bool initial_reservation){
	//send_term_debug_log_msg(_handle_track_handle_reservations)
	bool bool_result =_reserve_tracks_from_point(res_queue,train_num,front_data->node,front_data->offset, stopping_distance,initial_reservation);
	_release_track_from_point_behind(res_queue, train_num, back_data->node,back_data->offset, stopping_distance);
	return bool_result;
}
bool _track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance, bool initial_reservation) {
	ASSERT(res_queue != NULL);
	ASSERT(front_data != NULL && back_data != NULL);
	ASSERT(front_data->node != NULL && back_data->node != NULL);
	track_res_msg_t message;

	message.type = TR_MAKE_RESERVATIONS;
	message.res_queue = res_queue;
	message.train_num = train_num;
	message.front_data = front_data;
	message.back_data = back_data;
	message.stopping_distance = stopping_distance;
	message.initial_reservation = initial_reservation;

	bool bool_result;
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&message, sizeof(track_res_msg_t),(char*)&bool_result,sizeof(bool));
	return bool_result;

	
}
bool track_handle_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance) {

	return _track_handle_reservations( res_queue, train_num, front_data, back_data,  stopping_distance,false);

	
}
bool track_intial_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance) {

	return _track_handle_reservations( res_queue, train_num, front_data, back_data,  stopping_distance,true);
}

bool track_check_reservations_with_switch(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data, int stopping_distance, int switch_num) {
	ASSERT(res_queue != NULL);
	ASSERT(front_data->node != NULL );
	ASSERT(is_valid_switch_number(switch_num));

	track_res_msg_t message;

	message.type = TR_CHECK_RESERVATIONS_WITH_SWITCH;
	message.res_queue = res_queue;
	message.train_num = train_num;
	message.front_data = front_data;
	message.stopping_distance = stopping_distance;
	message.switch_num = switch_num;

	bool bool_result;
	Send(TRACK_RESERVATION_SERVER_ID,(char*)&message, sizeof(track_res_msg_t),(char*)&bool_result,sizeof(bool));
	return bool_result;

	
}



void _print_reserved_tracks(reserved_node_queue_t* res_queue,int train_num){
	return;
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
bool _handle_recursive_release_nodes(reserved_node_queue_t* res_queue,track_node* our_node, int train_num) {
	if(our_node == NULL || our_node->reserved_by == -1) return true;
	if(our_node->reserved_by != train_num || our_node->type == NODE_EXIT) return false;
	bool changed = false;
	//Burn the bridge behind us so we can't recurse back on ourselves
	ASSERT(our_node->edge[DIR_AHEAD].touched == true || our_node->edge[DIR_AHEAD].touched == false);
	//send_term_debug_log_msg("recursing on %s for tr %d", our_node->name, train_num);

	if(our_node->type == NODE_BRANCH){
		if(!our_node->edge[DIR_AHEAD].touched){
			track_touch_edge(our_node->edge+DIR_AHEAD,true);
			_handle_recursive_release_nodes(res_queue,our_node->edge[DIR_AHEAD].dest, train_num);
			changed = true;
		}
		if(!our_node->edge[DIR_CURVED].touched){
			track_touch_edge(our_node->edge+DIR_CURVED,true);
			_handle_recursive_release_nodes(res_queue,our_node->edge[DIR_CURVED].dest, train_num);
			changed = true;
		}
		track_touch_edge(our_node->edge+DIR_AHEAD,false);
		track_touch_edge(our_node->edge+DIR_CURVED,false);
	}else if(our_node->type == NODE_MERGE) {
		if(!our_node->edge[DIR_AHEAD].touched){

			ASSERT(our_node->reverse->type == NODE_BRANCH);
			//Trac
			track_touch_edge(our_node->edge+DIR_AHEAD,true);
			_handle_recursive_release_nodes(res_queue,our_node->reverse,train_num);
			_handle_recursive_release_nodes(res_queue,our_node->edge[DIR_AHEAD].dest,train_num);
			track_touch_edge(our_node->edge+DIR_AHEAD,false);
			changed = true;
		}
	} else {
		if(!our_node->edge[DIR_AHEAD].touched){
			track_touch_edge(our_node->edge+DIR_AHEAD,true);
			_handle_recursive_release_nodes(res_queue,our_node->edge[DIR_AHEAD].dest, train_num);
			track_touch_edge(our_node->edge+DIR_AHEAD,false);
			changed = true;
		}
	}

	if(changed){
		_handle_node_release(res_queue, our_node,train_num);
		//RESERVED_REMOVE_VALUE(*res_queue,our_node);
	}	
	return changed;
}

void _check_reservation_constraints(reserved_node_queue_t* res_queue,int train_num){
	track_node* adj_iterator;
	track_node* res_iterator;
	
	for(res_iterator = res_queue->head;res_iterator!=NULL;res_iterator = res_iterator->next_reserved){
		for(adj_iterator = res_queue->head;adj_iterator!=NULL;adj_iterator = adj_iterator->next_reserved){
			if(_is_track_node_adacent(res_iterator,adj_iterator ) ){
				break;
			}
			
		}
		send_term_debug_log_msg("Found a lone node");
		ASSERT(0);
	}
	send_term_debug_log_msg("Reservations are sparse");
	_print_reserved_tracks(res_queue,train_num);
	ASSERT(0);

}
