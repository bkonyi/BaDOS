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
#include <io/uart_notifier.h>
#include <io/uart_courrier.h>
#include <idle.h>
#include <tests/test_task.h>

#include <a3_user_prog.h>

#define SET_DELAY_INFO(delay, iterations) (((uint64_t)(delay)) << 32) | ((uint32_t)(iterations))

void first_user_task(void) {
    //bwprintf(COM2, "Starting the Name Server with priority %d.\r\n",SCHEDULER_HIGHEST_PRIORITY);
    //IMPORTANT:
    //The nameserver need to be the first task created so that is has TID of NAMESERVER_TID
    tid_t tid = Create(SCHEDULER_HIGHEST_PRIORITY,  nameserver_task);
    //bwprintf(COM2, "Created Name Server with tid %d.\r\n",tid);
    ASSERT(tid == NAMESERVER_TID);

    //Create the clock server
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, clock_server_task);
    //Create the random number generation server
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, rand_server_task);
    //Create the idle task which keeps the kernel running even when every other task is blocked.
    Create(SCHEDULER_LOWEST_PRIORITY, idle_task);

    /*Create(SCHEDULER_HIGHEST_PRIORITY - 3, delay_task);
    Create(SCHEDULER_HIGHEST_PRIORITY - 4, delay_task);
    Create(SCHEDULER_HIGHEST_PRIORITY - 5, delay_task);
    Create(SCHEDULER_HIGHEST_PRIORITY - 6, delay_task);

    int receive_tid;
    uint64_t delay_info;

    Receive(&receive_tid, NULL, 0);
    delay_info = SET_DELAY_INFO(10, 20);
    Reply(receive_tid, (char*)&delay_info, sizeof(uint64_t));

    Receive(&receive_tid, NULL, 0);
    delay_info = SET_DELAY_INFO(23, 9);
    Reply(receive_tid, (char*)&delay_info, sizeof(uint64_t));

    Receive(&receive_tid, NULL, 0);
    delay_info = SET_DELAY_INFO(33, 6);
    Reply(receive_tid, (char*)&delay_info, sizeof(uint64_t));


    Receive(&receive_tid, NULL, 0);
    delay_info = SET_DELAY_INFO(71, 3);
    Reply(receive_tid, (char*)&delay_info, sizeof(uint64_t));*/

    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_transmit_server);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_transmit_notifier);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_transmit_server);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_transmit_notifier);

    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_receive_notifier);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_receive_server);
    
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_receive_server);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_receive_notifier);


    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_timeout_notifier);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_timeout_notifier);

    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart_courrier);


    Create(SCHEDULER_LOWEST_PRIORITY + 1 , test_task);

    Exit();
}
