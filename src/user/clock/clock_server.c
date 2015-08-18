#include <clock/clock_server.h>
#include <clock/clock_notifier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>
#include <bwio.h>
#include <queue.h>
#include <global.h>
#include <ring_buffer.h>
#include <task_priorities.h>
typedef struct delayed_task_t{
    tid_t tid;
    uint32_t ticks;
    struct delayed_task_t* NEXT_DELAY_TASK;
}delayed_task_t;

#define DELAY_QUEUE_PUSH_BACK(Q, INPUT) QUEUE_PUSH_BACK_GENERIC(Q, INPUT, NEXT_DELAY_TASK)

#define DELAY_QUEUE_POP_FRONT(Q, VALUE) QUEUE_POP_FRONT_GENERIC(Q, VALUE, NEXT_DELAY_TASK)
#define DELAY_QUEUE_SORTED_INSERT(Q,INPUT) QUEUE_SORTED_INSERT(Q,INPUT,NEXT_DELAY_TASK,ticks, <=)

CREATE_QUEUE_TYPE(delay_queue_t, delayed_task_t);
CREATE_QUEUE_TYPE(available_delayed_tasks_t, delayed_task_t);

void delay_queue_insert(delay_queue_t * queue,available_delayed_tasks_t* available_delayed_tasks_queue,tid_t tid, uint32_t ticks);
delayed_task_t * delay_queue_pop(delay_queue_t * queue,available_delayed_tasks_t* available_delayed_tasks);

void clock_server_task(void) {
    int32_t ticks = 0;
    uint32_t CLOCK_NOTIFIER_TID;
    int32_t receive_tid;
    int result;
    clock_server_msg_t message;
    delay_queue_t delay_queue;
    available_delayed_tasks_t available_delayed_tasks_queue;

    delayed_task_t * next_delay_task = NULL;
    delayed_task_t task_delay_heap[MAX_NUMBER_OF_TASKS];
    
    QUEUE_INIT(delay_queue);
    QUEUE_INIT(available_delayed_tasks_queue);

    //Fill the queue with all of the available pointers to delayed_task_t
    //inside our delayed_task_t heap
    uint32_t i;
    for(i=0; i < MAX_NUMBER_OF_TASKS; i++) {
       DELAY_QUEUE_PUSH_BACK(available_delayed_tasks_queue, &(task_delay_heap[i]));
    }

    //We are the CLOCK_SERVER
    RegisterAs(CLOCK_SERVER);

    CLOCK_NOTIFIER_TID = CreateName(CLOCK_NOTIFIER_TASK_PRIORITY, clock_notifier_task, CLOCK_NOTIFIER);

    //NOTE: each tick is 10 milli seconds
    FOREVER {
        //Receive messages from the clock_notifier_task or tasks that are asking
        // for Delay,DelayUntil or Time
        Receive(&receive_tid, (char*)(&message), sizeof(clock_server_msg_t));

        //Check to see if the clock notifier has detected a tick.
        if(receive_tid == CLOCK_NOTIFIER_TID) {
            ++ticks;
           
            Reply(CLOCK_NOTIFIER_TID, (char*)NULL, 0); // Let the clock notifier know we have received it's message
            //Check to see if we have reached the necesarry ticks for the next task
            //that has been delayed
            while(next_delay_task != NULL && next_delay_task->ticks <= ticks) {

                result = ticks;
                //bwprintf(COM2,"CURTICKS: %d reply to tid: %d for ticks: %d\r\n",ticks,next_delay_task->tid, next_delay_task->ticks);
                Reply(next_delay_task->tid,(char*)&result,sizeof(int));
                //The current delay_task_t object is now free and can be added
                //to the available ones
                DELAY_QUEUE_PUSH_BACK(available_delayed_tasks_queue,next_delay_task);

                next_delay_task = delay_queue_pop(&delay_queue,&available_delayed_tasks_queue);
            }
            continue;
        }

        switch(message.type) {
            case TIME:
                Reply(receive_tid, (char*)&ticks, sizeof(int32_t));
                break;
            case DELAY:
                message.ticks +=ticks;
                //NOTE: there is intentionally no break here
            case DELAY_UNTIL:
                if(message.ticks <= ticks) {
                    result = ticks;
                    Reply(next_delay_task->tid,(char*)&result,sizeof(int32_t));
                }
                if(next_delay_task == NULL) {    //We don't have any tasks delayed
                    //get a free delayed_task object and fill it for the next_delayed_task
                    DELAY_QUEUE_POP_FRONT(available_delayed_tasks_queue,next_delay_task);
                    next_delay_task->ticks = message.ticks;
                    next_delay_task->tid = receive_tid;
                } else if(next_delay_task != NULL && message.ticks <= next_delay_task->ticks ) {
                    //the task asking to be delayed is delayed before any of our other delayed tasks
                    //put our current task back on the delay_queue
                    DELAY_QUEUE_SORTED_INSERT(delay_queue,next_delay_task);
                    //fill up a new delay_task_t object with this new tasks info
                    DELAY_QUEUE_POP_FRONT(available_delayed_tasks_queue,next_delay_task);
                    next_delay_task->ticks = message.ticks;
                    next_delay_task->tid = receive_tid;

                } else {
                    //This task just needs to be inserted into the delay_queue
                    delay_queue_insert(&delay_queue,&available_delayed_tasks_queue, receive_tid,message.ticks);
                }
                break;
            default:
                //Not a task that message that we respond to, return -1
                result = -1;
                Reply(receive_tid, (char*)&result, sizeof(int32_t));
                break;
        }
    }
}

void delay_queue_insert(delay_queue_t * queue,available_delayed_tasks_t* available_delayed_tasks,tid_t tid, uint32_t ticks) {
    delayed_task_t *free_spot;
    DELAY_QUEUE_POP_FRONT(*available_delayed_tasks,free_spot);
    free_spot->ticks = ticks;
    free_spot->tid   = tid;
    DELAY_QUEUE_SORTED_INSERT(*queue,free_spot);
}

delayed_task_t * delay_queue_pop(delay_queue_t * queue,available_delayed_tasks_t* available_delayed_tasks) {
    delayed_task_t *next_task;
    DELAY_QUEUE_POP_FRONT(*queue,next_task);
    return next_task;
}
