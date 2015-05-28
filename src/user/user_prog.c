#include <a1_user_prog.h>
#include <syscalls.h>
#include <bwio.h>
#include <global.h>
#include <nameserver.h>

#include <rand_server.h>
#include <rps/rps_server.h>
#include <rps/rps_client.h>

void first_user_task(void) {
    bwprintf(COM2, "Starting the Name Server with priority %d.\r\n",SCHEDULER_HIGHEST_PRIORITY);
    //IMPORTANT:
    //The nameserver need to be the first task created so that is has TID of NAMESERVER_TID
    tid_t tid = Create(SCHEDULER_HIGHEST_PRIORITY,  nameserver_task);
    bwprintf(COM2, "Created Name Server with tid %d.\r\n",tid);
    ASSERT(tid == NAMESERVER_TID);

    //Create the random number generation server
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, rand_server);

    //Creates the first user task.
    //NOTE: Priority chosen is arbitrary.
    Create(SCHEDULER_HIGHEST_PRIORITY - 2, rps_server_task);

    Create(SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task_3_plays);
    Create(SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task_3_plays);
    Create(SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task_5_plays);
    Create(SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task_3_plays);
    Create(SCHEDULER_HIGHEST_PRIORITY - 3, rps_client_task_3_plays);
    Exit();
}
