#ifndef __EVENTS_H__
#define __EVENTS_H__

#define NUMBER_OF_EVENTS 		64

//Event codes
#define TIMER3_EVENT  			51

#define UART1_RECEIVE_EVENT   	23
#define UART1_TRANSMIT_EVENT   	24
#define UART2_RECEIVE_EVENT   	25
#define UART2_TRANSMIT_EVENT   	26

#define UART1_EVENT 			52
#define UART2_EVENT 			54
#define UART1_STATUS_EVENT 		UART1_EVENT
#define UART2_STATUS_EVENT 		UART2_EVENT

#endif //__EVENTS_H__
