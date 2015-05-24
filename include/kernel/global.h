#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <common.h>
#include <queue.h>
#include <request.h>

/**
 *  TASKS DATA STRUCTURES
 */

#define MAX_NUMBER_OF_TASKS 1000 //TODO Arbitrary number for now
#define USER_TASK_MODE      0xD0
#define STACK_SIZE 1024 * 16 //16KiB

typedef int16_t tid_t;
typedef uint8_t priority_t;

typedef enum {
    TASK_RUNNING_STATE_ACTIVE           = 0,
    TASK_RUNNING_STATE_READY            = 1,
    TASK_RUNNING_STATE_ZOMBIE           = 2,
    TASK_RUNNING_STATE_REPLY_BLOCKED    = 3,
    TASK_RUNNING_STATE_RECEIVE_BLOCKED  = 4
} task_running_state_t;

struct task_descriptor_t;

#define MESSAGE_WAITING_NEXT next_message

CREATE_QUEUE_TYPE(message_waiting_queue_t, struct task_descriptor_t);

#define GET_NEXT_MESSAGE(Q, VALUE) {                            \
    QUEUE_POP_FRONT_GENERIC(Q, VALUE, MESSAGE_WAITING_NEXT);    \
} while(0)

#define QUEUE_MESSAGE(Q, VALUE) {                               \
    QUEUE_PUSH_BACK_GENERIC(Q, VALUE, MESSAGE_WAITING_NEXT);    \
} while(0)

#define IS_MESSAGE_WAITING(Q) (!IS_QUEUE_EMPTY(Q))

typedef struct task_descriptor_t {
    //Registers
    uint32_t sp;   //Stack pointer
    uint32_t spsr; //Program status register
    uint32_t pc;
    uint32_t return_code;

    struct task_descriptor_t* next;
    struct task_descriptor_t* MESSAGE_WAITING_NEXT;

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

#define SCHEDULER_NUM_QUEUES            32
#define SCHEDULER_HIGHEST_PRIORITY      31
#define SCHEDULER_LOWEST_PRIORITY       0

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
 *  GLOBAL DATA STRUCTURE
 */

typedef struct global_data_t {
    task_handler_data_t task_handler_data;
    scheduler_data_t scheduler_data;
} global_data_t;

#endif //__GLOBAL_H__
