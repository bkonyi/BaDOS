#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <common.h>
#include <queue.h>
#include <request.h>
#include <events.h>

/**
 *  TASKS DATA STRUCTURES
 */

#define MAX_NUMBER_OF_TASKS 200 //TODO Arbitrary number for now
#define USER_TASK_MODE      0x50
#define STACK_SIZE 1024 * 80 //80KiB

typedef enum {
    TASK_RUNNING_STATE_ACTIVE              = 0,
    TASK_RUNNING_STATE_READY               = 1,
    TASK_RUNNING_STATE_ZOMBIE              = 2,
    TASK_RUNNING_STATE_REPLY_BLOCKED       = 3,
    TASK_RUNNING_STATE_RECEIVE_BLOCKED     = 4,
    TASK_RUNNING_STATE_AWAIT_EVENT_BLOCKED = 5
} task_running_state_t;

struct task_descriptor_t;

//Define the message queue structures
#define MESSAGE_WAITING_NEXT next_message

CREATE_QUEUE_TYPE(message_waiting_queue_t, struct task_descriptor_t);

#define GET_NEXT_MESSAGE(Q, VALUE) {                            \
    QUEUE_POP_FRONT_GENERIC(Q, VALUE, MESSAGE_WAITING_NEXT);    \
} while(0)

#define QUEUE_MESSAGE(Q, VALUE) {                               \
    QUEUE_PUSH_BACK_GENERIC(Q, VALUE, MESSAGE_WAITING_NEXT);    \
} while(0)

#define IS_MESSAGE_WAITING(Q) (!IS_QUEUE_EMPTY(Q))

//Define the event waiting queue structures
#define NEXT_WAITING_TASK next_waiting_task

CREATE_QUEUE_TYPE(interrupt_waiting_tasks_queue_t, struct task_descriptor_t);

#define GET_NEXT_WAITING_TASK(Q, VALUE) {                    \
    QUEUE_POP_FRONT_GENERIC(Q, VALUE, NEXT_WAITING_TASK);    \
} while(0)

#define QUEUE_WAITING_TASK(Q, VALUE) {                       \
    QUEUE_PUSH_BACK_GENERIC(Q, VALUE, NEXT_WAITING_TASK);    \
} while(0)

#define ARE_TASKS_WAITING(Q) (!IS_QUEUE_EMPTY(Q))

typedef struct task_descriptor_t {
    //Registers
    uint32_t sp;   //Stack pointer
    uint32_t spsr; //Program status register
    uint32_t pc;
    uint32_t return_code;

    struct task_descriptor_t* next;
    struct task_descriptor_t* MESSAGE_WAITING_NEXT;
    struct task_descriptor_t* NEXT_WAITING_TASK;
    struct task_descriptor_t* NEXT_DELAY_TASK;

    char stack[STACK_SIZE];
    task_running_state_t state;
    tid_t tid;
    tid_t parent;
    priority_t priority;

    request_t* last_request;
    message_waiting_queue_t message_queue;

} task_descriptor_t;

typedef struct {
    task_descriptor_t tasks[MAX_NUMBER_OF_TASKS];
    tid_t next_tid;
} task_handler_data_t;

/**
 *  SCHEDULER DATA STRUCTURES
 */

CREATE_QUEUE_TYPE(schedule_queue_t, task_descriptor_t);

/**
 * Structure necessary to represent a queue of task descriptors.
 */

typedef struct scheduler_data_t {
    uint32_t occupied_queues;
    schedule_queue_t queues[SCHEDULER_NUM_QUEUES];
    task_descriptor_t* active_task;
} scheduler_data_t;

/**
 * SYSCALL HANDLER DATA STRUCTURES
 */
 typedef struct syscall_handler_data_t {
        interrupt_waiting_tasks_queue_t interrupt_waiting_tasks[NUMBER_OF_EVENTS];
 } syscall_handler_data_t;

/**
 *  GLOBAL DATA STRUCTURE
 */

typedef struct global_data_t {
    task_handler_data_t task_handler_data;
    scheduler_data_t scheduler_data;
    syscall_handler_data_t syscall_handler_data;
} global_data_t;


#endif //__GLOBAL_H__
