#include <task_handler.h>
#include <scheduler.h>
#include "common.h"
#include <bwio.h>

void init_task_handler(global_data_t* global_data) {
    global_data->task_handler_data.next_tid = 0;
}

int create_task(global_data_t* global_data, priority_t priority, void (*code) ()) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;

    if(task_handler_data->next_tid >= MAX_NUMBER_OF_TASKS) {
        //We're out of task descriptors!
        return -2;
    }

    if(priority > SCHEDULER_HIGHEST_PRIORITY) {
        //Invalid priority
        return -1;
    }

    task_descriptor_t* active_task = get_active_task(global_data);

    task_descriptor_t* next_descriptor = &(task_handler_data->tasks[task_handler_data->next_tid]);
    next_descriptor->state   = TASK_RUNNING_STATE_READY;
    next_descriptor->tid     = task_handler_data->next_tid;

    if(active_task != NULL) {
        next_descriptor->parent  = active_task->tid;
    } else {
        next_descriptor->parent = -1; //The kernel is our parent
    }

    uint32_t fp = (uint32_t) next_descriptor->stack + STACK_SIZE;

    //The stack grows downward, so change the offset
    //TODO: change 20 to the number of eleemnts we are going to save on the stack
    next_descriptor->sp      = fp - (20*sizeof(uint32_t)) ;
    next_descriptor->spsr    = USER_TASK_MODE;
    next_descriptor->pc      = (uint32_t) code;
    next_descriptor->priority = priority;

    int result = schedule(global_data, next_descriptor);

    if(result != 0) {
        //TODO handle scheduling error here, even though this shouldn't happen...
        return result;
    }

    return task_handler_data->next_tid++;
}

task_descriptor_t* get_task(global_data_t* global_data, tid_t tid) {
    return &(global_data->task_handler_data.tasks[tid]);
}
