#ifndef __CLOCK_SERVER_H__
#define __CLOCK_SERVER_H__

#include <common.h>

/**
 * The valid requests that can be made of the name server 
 */
typedef enum {
    TIME        = 0,    //Return the current number of ticks since kernel start
    DELAY       = 1,    //Delay execution of task for certain number of ticks
    DELAY_UNTIL = 2     //Resume execution of a task after a certain time has passed
} clock_server_msg_type_t;

/**
 * The message struct used to communicate with the clock server
 */
typedef struct {
    clock_server_msg_type_t type;	//The type of message being sent
    int32_t ticks;					//number of ticks to consider for the type case
} clock_server_msg_t;

/**
 * @brief The user task which handles clock functionality, including Time(), Delay(), and DelayUntil()
 * @details The user task which handles clock functionality, including Time(), Delay(), and DelayUntil()
 */
void clock_server_task(void);

#endif //__CLOCK_SERVER_H__
