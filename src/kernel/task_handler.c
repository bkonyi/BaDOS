#include <task_handler.h>
#include <scheduler.h>
#include "common.h"
#include <bwio.h>

#define NUMBER_USER_REGS_ON_STACK 12

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

    //Calculate the value of the top of the stack. We subtract 4 so we don't invade memory of another TD.
    uint32_t fp = (uint32_t) next_descriptor->stack + STACK_SIZE - 4;

    //The stack grows downward, so change the offset
    next_descriptor->sp      = fp - (NUMBER_USER_REGS_ON_STACK*sizeof(uint32_t));
    next_descriptor->spsr    = USER_TASK_MODE;
    next_descriptor->pc      = (uint32_t) code;
    next_descriptor->priority = priority;

    //Initialize the message queue
    QUEUE_INIT(next_descriptor->message_queue);

    int result = schedule(global_data, next_descriptor);

    KASSERT(result);

    //Return our TID and and then increment next_tid
    return task_handler_data->next_tid++; 
}

task_descriptor_t* get_task(global_data_t* global_data, tid_t tid) {
    return &(global_data->task_handler_data.tasks[tid]);
}

int is_valid_task(global_data_t* global_data, tid_t tid) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;

    //Check to see if the tid is within the valid range of tids
    if(tid >= MAX_NUMBER_OF_TASKS) {
        return -1;
    }

    //Check to see if the tid has been allocated to a task yet
    if(tid >= task_handler_data->next_tid) {
        return -2;
    }

    return 0;
}
