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
#define SET_DELAY_INFO(delay, iterations) (((uint64_t)(delay)) << 32) | ((uint32_t)(iterations))

void first_user_task(void) {
    /*******************************************************************/
    /*WARNING: DO NOT CHANGE ORDER OF TASKS BEFORE THE NOTE FOUND BELOW*/
    /*******************************************************************/

    //IMPORTANT:
    //The nameserver need to be the first task created so that is has TID of NAMESERVER_TID
    tid_t tid = Create(NAME_SERVER_PRIORITY,  nameserver_task);
    ASSERT(tid == NAMESERVER_TID);

    //Create the clock server
    Create(CLOCK_SERVER_PRIORITY, clock_server_task);
    //Create the random number generation server
    Create(RAND_SERVER_PRIORITY, rand_server_task);
    //Create the idle task which keeps the kernel running even when every other task is blocked.
    Create(IDLE_TASK_PRIORITY, idle_task);

    //Create the IO servers
    tid = Create(UART1_RECEIVE_SERVER_PRIORITY, uart1_receive_server);
    ASSERT(tid == UART1_RECEIVE_SERVER_ID);
    tid = Create(UART1_TRANSMIT_SERVER_PRIORITY, uart1_transmit_server);
    ASSERT(tid == UART1_TRANSMIT_SERVER_ID);
    tid = Create(UART2_RECEIVE_SERVER_PRIORITY, uart2_receive_server);
    ASSERT(tid == UART2_RECEIVE_SERVER_ID);
    tid = Create(UART2_TRANSMIT_SERVER_PRIORITY, uart2_transmit_server);
    ASSERT(tid == UART2_TRANSMIT_SERVER_ID);

    tid = Create(COMMAND_SERVER_PRIORITY, command_server);
    ASSERT(tid == COMMAND_SERVER_ID);

    tid = Create(TERMINAL_SERVER_PRIORITY, terminal_server);
    ASSERT(tid == TERMINAL_SERVER_ID);

    tid = Create(TRAIN_CONTROLLER_COMMAND_SERVER, train_controller_commander_server);
    ASSERT(tid == TRAIN_CONTROLLER_SERVER_ID);
    /*******************************************************************/
    /*        NOTE: CODE BELOW THIS POINT IS SAFE TO BE REORDERED      */
    /*******************************************************************/
    //tid = Create(SCHEDULER_LOWEST_PRIORITY + 1 , test_task);


    Exit();
}
