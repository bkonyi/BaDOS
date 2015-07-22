#ifndef _RESERVATION_SERVER_H_
#define _RESERVATION_SERVER_H_
#include <track_node.h>
typedef enum track_res_msg_type_t {
	TR_INIT 		= 1,
	TR_RESERVE,
	TR_RESERVE_REJECT,
	TR_RESERVE_APPROVE,
	TR_RELEASE,
	TR_UNSET
}track_res_msg_type_t;

typedef struct track_res_msg_t {
	track_res_msg_type_t type;
	track_node* node;
	int train_num;
} track_res_msg_t;

void track_reservation_server(void);
bool track_reserve_node(track_node* node,int train_num);
void track_release_node(track_node* node,int train_num);
void track_reservation_init(track_node* base_node);
bool track_handle_reservations(int train_num, track_node* our_node, int offset_in_node, int stopping_distance);
#endif// _RESERVATION_SERVER_H_

