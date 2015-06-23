#ifndef _TRACK_POSITION_SERVER_H_
#define _TRACK_POSITION_SERVER_H_
#include <common.h>

typedef enum tps_command_t {
	TPS_INITIALIZSE_TRAIN 	= 1,
	TPS_REMOVE_TRAIN 		= 2
} tps_command_t;



typedef struct tps_cover_sheet_t {
	tps_command_t command;
	uint32_t num1;
} tps_cover_sheet_t; // Sorry i couldn't resist making this office space reference.

typedef tps_cover_sheet_t tps_message_t;

void track_position_server(void);
#endif //_TRACK_POSITION_SERVER_H_

