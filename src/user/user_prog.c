#include <a1_user_prog.h>
#include <syscalls.h>
#include <bwio.h>
#include <global.h>
#include <nameserver.h>
void first_user_task(void) {
    bwprintf(COM2, "Starting the Name Server with priority %d.\r\n",SCHEDULER_HIGHEST_PRIORITY);
    //IMPORTANT:
    //The nameserver need to be the first task created so that is has TID of NAMESERVER_TID
    tid_t tid = Create(SCHEDULER_HIGHEST_PRIORITY,  nameserver_task);
    bwprintf(COM2, "Created Name Server with tid %d.\r\n",tid);
    ASSERT(tid == NAMESERVER_TID);
    Exit();
}
