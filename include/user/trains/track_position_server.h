#ifndef _TRACK_POSITION_SERVER_H_
#define _TRACK_POSITION_SERVER_H_
#include <common.h>
#include <track_data.h>
#include <queue.h>
//Set some macros to distinguish the 2 tracks in TPS cover sheet messages for TPS_SET_TRACK
#define TRACKA    0x14
#define TRACKB 	  0x41

typedef enum tps_command_t {
	TPS_ADD_TRAIN 	     = 1,
	TPS_REMOVE_TRAIN     = 2,
	TPS_SET_TRACK		 = 3,
	TPS_SWITCH_SET		 = 4,
	TPS_SET_TRAIN_SENSOR = 5
} tps_command_t;

typedef struct train_information_t {
	track_node* track;
	uint32_t train_num;
	tid_t	server_tid;

	struct train_information_t* next;
}train_information_t;


typedef struct tps_cover_sheet_t {
	tps_command_t command;
	uint32_t num1;
	uint32_t num2;
} tps_cover_sheet_t; // Sorry i couldn't resist making this office space reference.

typedef tps_cover_sheet_t tps_message_t;

CREATE_QUEUE_TYPE(tps_tpi_queue_t,train_information_t);

void track_position_server(void);

//Functions for making calls to the server

/**
 * @brief Sets the TPS so it is operating on a given track.
 * 
 * @param track should only ever be the TRACKA or TRACKB macro
 */
void tps_set_track(uint32_t track);
void tps_add_train(uint32_t train_num);
track_node* tps_set_train_sensor(uint32_t train_num, uint32_t sensor);
void tps_send_sensor_data(int8_t* sensors);
void tps_set_switch(uint32_t sw, char state);
#endif //_TRACK_POSITION_SERVER_H_

