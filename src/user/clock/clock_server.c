#include <clock/clock_server.h>
#include <clock/clock_notifier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>
#include <bwio.h>
#include <priority_queue.h>
#include <global.h>
#include <ring_buffer.h>

typedef struct delayed_task_t{
    tid_t tid;
    uint32_t ticks;
    struct delayed_task_t* NEXT_DELAY_TASK;
}delayed_task_t;

CREATE_PRIORITY_QUEUE_TYPE(clock_priority_queue_t, delayed_task_t);
CREATE_RING_BUFFER_TYPE(free_task_pointer_ring_buffer_t, delayed_task_t, MAX_NUMBER_OF_TASKS);

void delay_queue_insert(clock_priority_queue_t * queue,free_task_pointer_ring_buffer_t* free_task_pointers,tid_t tid, uint32_t ticks);

void clock_server_task(void) {
    int32_t ticks = 0;
    uint32_t CLOCK_NOTIFIER_TID;
    int32_t receive_tid;
    uint32_t result;
    clock_server_msg_t message;
    clock_priority_queue_t queue;
    delayed_task_t * next_delay_task;
    delayed_task_t task_delay_heap[MAX_NUMBER_OF_TASKS];
    free_task_pointer_ring_buffer_t free_task_pointers;

    RING_BUFFER_INIT(free_task_pointers,MAX_NUMBER_OF_TASKS);

    uint32_t i;
    for(i=0; i < MAX_NUMBER_OF_TASKS; i++){
        PUSH_BACK(free_task_pointers, &(task_delay_heap[i]),result);
    }

    POP_FRONT(free_task_pointers,next_delay_task);

    RegisterAs(CLOCK_SERVER);

    CLOCK_NOTIFIER_TID = Create(SCHEDULER_HIGHEST_PRIORITY, clock_notifier_task);

    FOREVER {
        Receive(&receive_tid, (char*)(&message), sizeof(clock_server_msg_t));

        //Check to see if the clock notifier has detected a tick.
        if(receive_tid == CLOCK_NOTIFIER_TID) {
            ++ticks;
            Reply(CLOCK_NOTIFIER_TID, (char*)NULL, 0);

            //TODO check to see if we need to respond to any Delay calls here
            continue;
        }

        switch(message.type) {
            case TIME:
                Reply(receive_tid, (char*)&ticks, sizeof(int32_t));
                break;
            case DELAY:
            case DELAY_UNTIL:
                delay_queue_insert(&queue,&free_task_pointers, receive_tid,message.ticks);
                break;
            default:
                result = -1;
                Reply(receive_tid, (char*)&result, sizeof(int32_t));
                break;
        }
    }
}

void delay_queue_insert(clock_priority_queue_t * queue,free_task_pointer_ring_buffer_t* free_task_pointers,tid_t tid, uint32_t ticks){
    delayed_task_t *free_spot;
    POP_FRONT(*free_task_pointers,free_spot);

    free_spot->ticks = ticks;
    free_spot->tid   = tid;
    PRIORITY_QUEUE_INSERT(*queue,free_spot,NEXT_DELAY_TASK,ticks);
}
