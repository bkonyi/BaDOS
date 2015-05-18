#include <scheduler.h>
#include <ring_buffer.h>

//This defines a special ring buffer structure for task_descriptor_t structs
CREATE_RING_BUFFER_TYPE(schedule_ring_buffer_t, task_descriptor_t, MAX_NUMBER_OF_TASKS)

static uint32_t occupied_queues = 0;
static schedule_ring_buffer_t queues[SCHEDULER_NUM_QUEUES];
static task_descriptor_t* active_task = NULL;

void init_scheduler(void) {
    int i;
    for(i = 0; i < SCHEDULER_NUM_QUEUES; ++i) {
        //Because C is stupid and we can't set default values for structs...
        queues[i].size = MAX_NUMBER_OF_TASKS;
    }
}

int schedule(task_descriptor_t* task) {
    if(task == NULL) {
        //Invalid task
        return -1;
    }

    if(task->priority > SCHEDULER_LOWEST_PRIORITY) {
        //Invalid priority
        return -2;
    }

    //Add the task to the queue with defined priority
    int result = push_back(&queues[task->priority], task);

    if(result != 0) {
        //TODO 
    }

    occupied_queues |= 0x1 << task->priority;

    return 0;
}

task_descriptor_t* schedule_next_task(void) {
    if(active_task != NULL) {
        schedule(active_task);
    }

    //Finds the log2 of the occupied queues
    //Don't ask me how this works, I found it online
    const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
    const unsigned int S[] = {1, 2, 4, 8, 16};
    uint32_t v = occupied_queues;
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

    schedule_ring_buffer_t* queue = &queues[r];

    active_task = pop_front(queue);

    if(IS_BUFFER_EMPTY(queue)) {
        occupied_queues &= ~(0x1 << r);
    }

    return active_task;
}

task_descriptor_t* get_active_task(void) {
    return active_task;
}
