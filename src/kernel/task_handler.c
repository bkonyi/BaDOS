#include <task_handler.h>
#include <scheduler.h>
#include "common.h"
#include <bwio.h>

#define STACK_SIZE 1024 * 16 //16KiB

static task_descriptor_t tasks[MAX_NUMBER_OF_TASKS];

//Note: we never reassign tids, even if a task is dead.
static tid_t next_tid;

void init_task_handler(void) {
    next_tid = 0;
}

int create_task(priority_t priority, void (*code) ()) {
    if(next_tid >= MAX_NUMBER_OF_TASKS) {
        bwprintf(COM2, "ERROR: Next TID: %d\r\n", next_tid);
        //We're out of task descriptors!
        return -2;
    }

    if(priority > SCHEDULER_LOWEST_PRIORITY) {
        bwprintf(COM2, "ERROR: priority: %d\r\n", priority);
        //Invalid priority
        return -1;
    }

    task_descriptor_t* active_task = get_active_task();

    task_descriptor_t* next_descriptor = &tasks[next_tid];
    next_descriptor->state   = TASK_RUNNING_STATE_READY;
    next_descriptor->tid     = next_tid;

    if(active_task != NULL) {
        next_descriptor->parent  = active_task->tid;
    } else {
        next_descriptor->parent = -1; //The kernel is our parent
    }

    uint32_t fp = (uint32_t) kmalloc(STACK_SIZE);
    //The stack grows downward, so change the offset
    //TODO: change 20 to the number of eleemnts we are going to save on the stack
    next_descriptor->sp      = fp + (20*sizeof(uint32_t)) ;

    next_descriptor->spsr    = USER_TASK_MODE;
    next_descriptor->pc      = (uint32_t) code + HARDCODED_LOAD_OFFSET;

    bwprintf(COM2, "FP: 0x%x\r\n", fp);
    bwprintf(COM2, "Code: 0x%x\r\n", code);
    bwprintf(COM2, "Code: 0x%x\r\n", code + HARDCODED_LOAD_OFFSET);
    bwprintf(COM2, "PC: 0x%x\r\n", next_descriptor->pc);
    
    int result = schedule(next_descriptor);

    if(result != 0) {
        //TODO handle scheduling error here, even though this shouldn't happen...
        bwprintf(COM2, "Bad scheduling!\r\n");

        return result;
    }

    return next_tid++;
}

task_descriptor_t* get_task(tid_t tid) {
    return &tasks[tid];
}
