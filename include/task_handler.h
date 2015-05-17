#ifndef __TASK_HANDLER_H__
#define __TASK_HANDLER_H__

#include <common.h>

#define MAX_NUMBER_OF_TASKS 1000 //TODO Arbitrary number for now
#define USER_TASK_MODE      0xD0
typedef int16_t tid_t;

typedef uint8_t priority_t;

typedef enum {
    TASK_RUNNING_STATE_ACTIVE = 0,
    TASK_RUNNING_STATE_READY  = 1,
    TASK_RUNNING_STATE_ZOMBIE = 2
} task_running_state_t;

typedef struct {
    //Registers
    uint32_t sp;   //Stack pointer
    uint32_t spsr; //Program status register
    uint32_t pc;

    task_running_state_t state;
    tid_t tid;
    tid_t parent;
    priority_t priority;

} task_descriptor_t;

/**
 * @brief instantiate a task
 * @details Create allocates and initializes a task descriptor, using the
 * given priority, and the given function pointer as a pointer to the entry
 * point of executable code, essentially a function with no arguments and no
 * return value. When create_task returns the task descriptor has all the state
 * needed to run the task, the task’s stack has been suitably initialized, and
 * the task has been entered into its ready queue so that it will run the next
 * time it is scheduled.
 * 
 * @param priority [description]
 * @param code [description]
 * 
 * @return
 *  id – the positive integer task id of the newly created task. The task id 
 *      must be unique, in the sense that no task has, will have or has had the
 *      same task id.
 *  -1 – if the priority is invalid.
 *  -2 – if the kernel is out of task descriptors.
 */
int create_task(priority_t priority, void (*code)());

#endif //__TASK_HANDLER_H__
