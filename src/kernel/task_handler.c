#include <task_handler.h>
#include <scheduler.h>
#include "common.h"
#include <bwio.h>
#include <task_priorities.h>

#define NUMBER_USER_REGS_ON_STACK 13

#define TID_INDEX(tid) ((uint16_t)((tid) & 0xFF))
#define TASK_GENERATION(tid) ((uint32_t)((tid) >> 8))
#define SET_TASK_GENERATION(tid, generation) (tid = (TID_INDEX(tid) | (generation << 8)))

void init_task_handler(global_data_t* global_data) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;
    task_handler_data->next_tid = 0;

    RING_BUFFER_INIT(task_handler_data->free_tasks, MAX_NUMBER_OF_TASKS);

    int i;
    for(i = 0; i < MAX_NUMBER_OF_TASKS; ++i) {
        task_descriptor_t* task = &task_handler_data->tasks[i];
        task->generational_tid = i;
    }
}

int create_task(global_data_t* global_data, priority_t priority, void (*code) (), char* name) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;

    if(priority > TASK_HIGHEST_PRIORITY) {
        //Invalid priority
        return -1;
    }

    task_descriptor_t* active_task = get_active_task(global_data);

    task_descriptor_t* next_descriptor;

    if(task_handler_data->next_tid == MAX_NUMBER_OF_TASKS) {

        if(IS_BUFFER_EMPTY(task_handler_data->free_tasks)) {
            //We're out of tasks
            KASSERT(0);
            return -2;
        }

        //Recycle a used task descriptor
        POP_FRONT(task_handler_data->free_tasks, next_descriptor);

    } else {
        //Allocate the next new task descriptor and increment the free tid counter
        next_descriptor = &(task_handler_data->tasks[task_handler_data->next_tid++]);
    }
    
    next_descriptor->state             = TASK_RUNNING_STATE_READY;
    next_descriptor->running_time      = 0;
    
    strlcpy(next_descriptor->task_name, name, MAX_TASK_NAME_SIZE);

    if(active_task != NULL) {
        next_descriptor->parent  = active_task->generational_tid;
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
    KASSERT(result == 0);

    //Return our generational TID
    return next_descriptor->generational_tid;
}

int destroy_task(global_data_t* global_data, int tid) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;

    uint16_t tid_index = TID_INDEX(tid);
    int result;

    //Check to see if the tid is valid
    if(tid_index >= MAX_NUMBER_OF_TASKS) {
        return -1;
    } 

    task_descriptor_t* destroyed_task = &task_handler_data->tasks[tid_index];
    task_descriptor_t* active_task = get_active_task(global_data);

    //Check to see if a task is trying to destroy itself.
    //This might cause problems when we try to reschedule so we'll error out.
    if(destroyed_task == active_task) {
        return -2;
    } else if(TASK_GENERATION(destroyed_task->generational_tid) != TASK_GENERATION(tid)) {
        //Check to see if the task was destroyed in a previous generation
        //or doesn't exist yet
        return -3;
    }

    destroyed_task->state = TASK_RUNNING_STATE_FREE;
    SET_TASK_GENERATION(destroyed_task->generational_tid, (TASK_GENERATION(destroyed_task->generational_tid) + 1));

    if(!(TASK_GENERATION(destroyed_task->generational_tid) >= (0x1 << 25))) {
        PUSH_BACK(task_handler_data->free_tasks, destroyed_task, result);
        KASSERT(result == 0);
    }

    return 0;
}

task_descriptor_t* get_task(global_data_t* global_data, tid_t tid) {
    return &(global_data->task_handler_data.tasks[TID_INDEX(tid)]);
}

int is_valid_task(global_data_t* global_data, tid_t tid) {
    task_handler_data_t* task_handler_data = &global_data->task_handler_data;

    //Check to see if the tid is within the valid range of tids
    if(TID_INDEX(tid) >= MAX_NUMBER_OF_TASKS) {
        return -1;
    }

    task_descriptor_t* task = &task_handler_data->tasks[TID_INDEX(tid)];

    //Check to see if the tid has been allocated to a task yet, or it is the wrong generation
    if(TASK_GENERATION(tid) != TASK_GENERATION(task->generational_tid)) {
        return -2;
    }

    return 0;
}
