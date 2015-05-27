#ifndef _RPS_SERVER_H_
#define _RPS_SERVER_H_

typedef enum{
	RPS_SIGNUP=1,
	RPS_PLAY,
	RPS_QUIT
} rps_msg_type;

typedef enum{
	ROCK=0x10,
	PAPER,
	SCISSORS
} rps_selection;

typedef struct {
	rps_msg_type type;
	rps_selection selection;
}rps_msg;
void rps_server_task(void);

#endif
