#include <a1_user_prog.h>
#include <syscalls.h>
#include <bwio.h>
#include <global.h>

void first_task(void) {
  /*  bwprintf(COM2, "First: Starting.\r\n");
    tid_t tid = Create(SCHEDULER_LOWEST_PRIORITY,  a1_user_task);
    bwprintf(COM2, "Created: %d\r\n", tid);

    tid = Create(SCHEDULER_LOWEST_PRIORITY,  a1_user_task);
    bwprintf(COM2, "Created: %d\r\n", tid);

    tid = Create(SCHEDULER_HIGHEST_PRIORITY, a1_user_task);
    bwprintf(COM2, "Created: %d\r\n", tid);

    tid = Create(SCHEDULER_HIGHEST_PRIORITY, a1_user_task);
    bwprintf(COM2, "Created: %d\r\n", tid);

    bwprintf(COM2, "First: Exiting.\r\n");*/
    Exit();
}

void a1_user_task(void) {
    tid_t my_tid, parent_tid;
    my_tid = MyTid();
    parent_tid = MyParentTid();

    bwprintf(COM2, "#1 Task ID: %d Parent ID: %d\r\n", my_tid, parent_tid);

    Pass();

    bwprintf(COM2, "#2 Task ID: %d Parent ID: %d\r\n", my_tid, parent_tid);

    Exit();
}
