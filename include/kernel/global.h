#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <common.h>
#include <queue.h>

/**
 *  TASKS DATA STRUCTURES
 */

#define MAX_NUMBER_OF_TASKS 1000 //TODO Arbitrary number for now
#define USER_TASK_MODE      0xD0
#define STACK_SIZE 1024 * 16 //16KiB

typedef int16_t tid_t;
typedef uint8_t priority_t;

typedef enum {
    TASK_RUNNING_STATE_ACTIVE = 0,
    TASK_RUNNING_STATE_READY  = 1,
    TASK_RUNNING_STATE_ZOMBIE = 2
} task_running_state_t;

typedef struct task_descriptor_t {
    //Registers
    uint32_t sp;   //Stack pointer
    uint32_t spsr; //Program status register
    uint32_t pc;
    uint32_t return_code;

    struct task_descriptor_t* next;
    char stack[STACK_SIZE];
    task_running_state_t state;
    tid_t tid;
    tid_t parent;
    priority_t priority;

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
