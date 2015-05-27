#ifndef __RPS_COMMON_H__
#define __RPS_COMMON_H__

typedef enum{
    RPS_SIGNUP = 0,
    RPS_PLAY   = 1,
    RPS_QUIT   = 2
} rps_msg_type;

typedef enum{
    ROCK       = 0,
    PAPER      = 1,
    SCISSORS   = 2,
} rps_selection;

typedef struct {
    rps_msg_type type;
    rps_selection selection;
} rps_msg;

#endif
