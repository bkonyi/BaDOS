#ifndef _TASK_PRIORITIES_H_
#define _TASK_PRIORITIES_H_
#include <common.h>
//KERNEL Tasks

#define NAME_SERVER_PRIORITY 			SCHEDULER_HIGHEST_PRIORITY
#define CLOCK_SERVER_PRIORITY			SCHEDULER_HIGHEST_PRIORITY - 1
#define RAND_SERVER_PRIORITY			SCHEDULER_HIGHEST_PRIORITY - 1
#define IDLE_TASK_PRIORITY				SCHEDULER_LOWEST_PRIORITY
#define UART1_RECEIVE_SERVER_PRIORITY	SCHEDULER_HIGHEST_PRIORITY - 1
#define UART1_TRANSMIT_SERVER_PRIORITY	SCHEDULER_HIGHEST_PRIORITY - 1
#define UART2_RECEIVE_SERVER_PRIORITY	SCHEDULER_HIGHEST_PRIORITY - 1
#define UART2_TRANSMIT_SERVER_PRIORITY	SCHEDULER_HIGHEST_PRIORITY - 1
#define COMMAND_SERVER_PRIORITY			SCHEDULER_HIGHEST_PRIORITY - 1
#define TERMINAL_SERVER_PRIORITY		SCHEDULER_HIGHEST_PRIORITY - 2	
#define TRAIN_CONTROLLER_COMMAND_SERVER	SCHEDULER_HIGHEST_PRIORITY - 1
//USER Tasks

#endif //_TASK_PRIORITIES_H_
