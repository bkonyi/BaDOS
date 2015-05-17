#include <task_handler.h>
#include <scheduler.h>

#define STACK_SIZE 1024 * 16 //16KiB

static task_descriptor_t tasks[MAX_NUMBER_OF_TASKS];

//Note: we never reassign tids, even if a task is dead.
static tid_t next_tid = 0;

int create_task(priority_t priority, void (*code) ()) {
    if(next_tid >= MAX_NUMBER_OF_TASKS) {
        //We're out of task descriptors!
        return -2;
    }

    if(priority < SCHEDULER_LOWEST_PRIORITY || 
        priority > SCHEDULER_HIGHEST_PRIORITY) {
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

    //The stack grows downward, so change the offset
    next_descriptor->sp      = (uint32_t) kmalloc(STACK_SIZE) + (STACK_SIZE);

    next_descriptor->spsr    = USER_TASK_MODE;
    next_descriptor->pc      = (uint32_t) code;
    
    int result = schedule(next_descriptor);

    if(result != 0) {
        //TODO handle scheduling error here, even though this shouldn't happen...
        return result;
    }

    return next_tid++;
}