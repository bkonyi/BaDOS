#include <scheduler.h>
#include <ring_buffer.h>
#include <bwio.h>

void init_scheduler(global_data_t* global_data) {
    scheduler_data_t* scheduler_data = &global_data->scheduler_data;

    scheduler_data->occupied_queues = 0;
    scheduler_data->active_task = NULL;

    int i;
    for(i = 0; i < SCHEDULER_NUM_QUEUES; ++i) {
        //Because C is stupid and we can't set default values for structs...
        scheduler_data->queues[i].size  = MAX_NUMBER_OF_TASKS;
        scheduler_data->queues[i].start = 0;
        scheduler_data->queues[i].end   = 0;
    }
}

int schedule(global_data_t* global_data, task_descriptor_t* task) {
    if(task == NULL) {
        //Invalid task
        return -1;
    }

    if(task->priority > SCHEDULER_HIGHEST_PRIORITY) {
        //Invalid priority
        return -2;
    }

    scheduler_data_t* scheduler_data = &global_data->scheduler_data;

    //Add the task to the queue with defined priority
    int result;
    PUSH_BACK((scheduler_data->queues[task->priority]), task, result);

    if(result != 0) {
        //TODO 
    }

    bwprintf(COM2, "Priority: %d\r\n", task->priority);

    scheduler_data->occupied_queues |= 0x1 << task->priority;

    bwprintf(COM2, "Occupied Queues: 0x%x\r\n", scheduler_data->occupied_queues);

    return 0;
}

task_descriptor_t* schedule_next_task(global_data_t* global_data) {
    scheduler_data_t* scheduler_data = &global_data->scheduler_data;
    task_descriptor_t* active_task = scheduler_data->active_task;

    if(active_task != NULL && active_task->state == TASK_RUNNING_STATE_ACTIVE) {
        //TODO add this back in
        //schedule(global_data, scheduler_data->active_task);
    }

    //Finds the log2 of the occupied queues
    //Don't ask me how this works, I found it online
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const unsigned int S[] = {1, 2, 4, 8, 16};

    uint32_t v = scheduler_data->occupied_queues;

    int i;

    register unsigned int r = 0; // result
    for (i = 4; i >= 0; i--)
    {
      if (v & b[i])
      {
        v >>= S[i];
        r |= S[i];
      }
    }
    //End log2 finding

    POP_FRONT(scheduler_data->queues[r], scheduler_data->active_task);

    if(IS_BUFFER_EMPTY(scheduler_data->queues[r])) {
        scheduler_data->occupied_queues &= ~(0x1 << r);
    }

    return scheduler_data->active_task;
}

task_descriptor_t* get_active_task(global_data_t* global_data) {
    return global_data->scheduler_data.active_task;
}

void zombify_active_task(global_data_t* global_data) {
    task_descriptor_t* active_task = global_data->scheduler_data.active_task;
    
    if(active_task != NULL) {
        active_task->state = TASK_RUNNING_STATE_ZOMBIE;
    }
}
