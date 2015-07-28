#ifndef _RESERVATION_SERVER_H_
#define _RESERVATION_SERVER_H_
#include <track_node.h>
#include <queue.h>
CREATE_QUEUE_TYPE(reserved_node_queue_t, track_node);

bool track_compare_reserved_node(track_node* a, track_node* b);

#define RESERVED_PUSH_BACK(Q,VALUE) QUEUE_PUSH_BACK_GENERIC(Q,VALUE,next_reserved)
#define RESERVED_POP_FRONT(Q,VALUE) QUEUE_POP_FRONT_GENERIC(Q,VALUE,next_reserved)
#define RESERVED_VALUE_EXISTS_IN(Q,VALUE,RETVAL) QUEUE_VALUE_EXISTS_IN(Q,VALUE,RETVAL,next_reserved)
#define RESERVED_REMOVE_VALUE(Q,INPUT) QUEUE_REMOVE_VALUE(Q, INPUT, next_reserved, track_compare_reserved_node)
typedef enum track_res_msg_type_t {
	TR_INIT 		= 1,
	TR_RESERVE,
	TR_RESERVE_REJECT,
	TR_RESERVE_APPROVE,
	TR_MAKE_RESERVATIONS,
	TR_RELEASE,
	TR_UNSET,
	TR_CHECK_RESERVATIONS_WITH_SWITCH
}track_res_msg_type_t;

typedef struct track_res_msg_t {
	track_res_msg_type_t type;
	reserved_node_queue_t* res_queue;
	int train_num;
	track_node_data_t* front_data;
	track_node_data_t* back_data;
	track_node* our_node;
	int stopping_distance;
	bool initial_reservation;
	int switch_num;
} track_res_msg_t;

void track_reservation_server(void);
bool track_reserve_node(reserved_node_queue_t* res_queue, track_node* node,int train_num);
void track_release_node(track_node* node,int train_num);
void track_reservation_init(track_node* base_node);
bool track_handle_reservations(reserved_node_queue_t* res_queue, int train_num,track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance) ;
bool track_intial_reservations(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data,track_node_data_t* back_data, int stopping_distance);
void track_clear_reservations(reserved_node_queue_t* res_queue, int train_num, track_node* our_node, int offset_in_node, int stopping_distance);
bool track_check_reservations_with_switch(reserved_node_queue_t* res_queue, int train_num, track_node_data_t* front_data, int stopping_distance, int switch_num);

#endif// _RESERVATION_SERVER_H_

