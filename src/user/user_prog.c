#include <a1_user_prog.h>
#include <syscalls.h>
#include <bwio.h>
#include <global.h>
#include <nameserver.h>

#include <rand_server.h>
#include <rps/rps_server.h>
#include <rps/rps_client.h>
#include <clock/clock_server.h>
#include <io/uart_servers.h>
#include <terminal/terminal.h>
#include <idle.h>
#include <tests/test_task.h>
#include <user/servers.h>
#include <trains/train_controller_commander.h>
#include <command_server.h>
#include <a3_user_prog.h>
#include <task_priorities.h>
#include <servers.h>
#include <trains/track_position_server.h>
#define SET_DELAY_INFO(delay, iterations) (((uint64_t)(delay)) << 32) | ((uint32_t)(iterations))

void first_user_task(void) {
    /*******************************************************************/
    /*WARNING: DO NOT CHANGE ORDER OF TASKS BEFORE THE NOTE FOUND BELOW*/
    /*******************************************************************/

    //IMPORTANT:
    //The nameserver need to be the first task created so that is has TID of NAMESERVER_TID
    tid_t tid = CreateName(NAME_SERVER_PRIORITY,  nameserver_task, NAME_SERVER);
    ASSERT(tid == NAMESERVER_TID);

    //Create the clock server
    tid = CreateName(CLOCK_SERVER_PRIORITY, clock_server_task, CLOCK_SERVER);
    ASSERT(tid == CLOCK_SERVER_ID);
    //Create the random number generation server
    CreateName(RAND_SERVER_PRIORITY, rand_server_task, RAND_SERVER);
    //Create the idle task which keeps the kernel running even when every other task is blocked.
    CreateName(IDLE_TASK_PRIORITY, idle_task, IDLE_TASK);

    //Create the IO servers
    tid = CreateName(UART1_RECEIVE_SERVER_PRIORITY, uart1_receive_server, UART1_RECEIVE_SERVER);
    ASSERT(tid == UART1_RECEIVE_SERVER_ID);
    tid = CreateName(UART1_TRANSMIT_SERVER_PRIORITY, uart1_transmit_server, UART1_TRANSMIT_SERVER);
    ASSERT(tid == UART1_TRANSMIT_SERVER_ID);
    tid = CreateName(UART2_RECEIVE_SERVER_PRIORITY, uart2_receive_server, UART2_RECEIVE_SERVER);
    ASSERT(tid == UART2_RECEIVE_SERVER_ID);
    tid = CreateName(UART2_TRANSMIT_SERVER_PRIORITY, uart2_transmit_server, UART2_TRANSMIT_SERVER);
    ASSERT(tid == UART2_TRANSMIT_SERVER_ID);

    tid = CreateName(COMMAND_SERVER_PRIORITY, command_server, COMMAND_SERVER);
    ASSERT(tid == COMMAND_SERVER_ID);

    tid = CreateName(TERMINAL_SERVER_PRIORITY, terminal_server, TERMINAL_SERVER);
    ASSERT(tid == TERMINAL_SERVER_ID);

    tid = CreateName(TRAIN_CONTROLLER_COMMAND_SERVER, train_controller_commander_server, TRAIN_CONTROLLER_SERVER);
    ASSERT(tid == TRAIN_CONTROLLER_SERVER_ID);
    tid = CreateName(TRAIN_POSITION_SERVER_PRIORITY, track_position_server, TRAIN_CONTROLLER_SERVER);
    ASSERT(tid == TRAIN_POSITION_SERVER_ID);
    
    /*******************************************************************/
    /*        NOTE: CODE BELOW THIS POINT IS SAFE TO BE REORDERED      */
    /*******************************************************************/
    //tid = Create(5 , test_task);


    Exit();
}
