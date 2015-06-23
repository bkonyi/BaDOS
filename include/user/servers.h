#ifndef __SERVERS_H__
#define __SERVERS_H__

#define NAME_SERVER                "NAME SERVER"
#define CLOCK_SERVER               "CLOCK_SERVER"
#define CLOCK_NOTIFIER             "CLOCK_NOTIFIER"
#define RAND_SERVER                "RAND_SERVER"
#define RPS_SERVER                 "RPS_SERVER"
#define IDLE_TASK                  "IDLE_TASK"
#define IDLE_TASK_ID               4

#define UART1_RECEIVE_NOTIFIER     "UART1_RECEIVE_NOTIFIER"
#define UART1_RECEIVE_SERVER       "UART1_RECEIVE_SVR"
#define UART1_RECEIVE_SERVER_ID    5
#define UART1_TRANSMIT_NOTIFIER    "UART1_TRANSMIT_NOTIFIER"
#define UART1_TRANSMIT_SERVER      "UART1_TRANSMIT_SVR"
#define UART1_TRANSMIT_SERVER_ID   6
#define UART2_RECEIVE_NOTIFIER     "UART2_RECEIVE_NOTIFIER"
#define UART2_TIMEOUT_NOTIFIER     "UART2_TIMEOUT_NOTIFIER"
#define UART2_RECEIVE_SERVER       "UART2_RECEIVE_SVR"
#define UART2_RECEIVE_SERVER_ID    7
#define UART2_TRANSMIT_NOTIFIER    "UART2_TRANSMIT_NOTIFIER"
#define UART2_TRANSMIT_SERVER      "UART2_TRANSMIT_SVR"
#define UART2_TRANSMIT_SERVER_ID   8
#define UART2_COURRIER             "UART2_COURRIER"

#define COMMAND_SERVER_ID		   9
#define COMMAND_SERVER 			   "COMMAND_SERVER"

#define TERMINAL_TICK_NOTIFIER     "TERMINAL_TICK_NOTFIER"
#define TERMINAL_SERVER            "TERMINAL_SERVER"
#define TERMINAL_SERVER_ID         10

#define TRAIN_REVERSE_DELAY_SERVER "TRAIN_REVERSE_DELAY_SERVER"
#define SENSOR_QUERY_SERVER        "SENSOR_QUERY_SERVER"
#define TRAIN_CONTROLLER_SERVER    "TRAIN_CTRL_SERVER"
#define TRAIN_CONTROLLER_SERVER_ID 11
#endif
