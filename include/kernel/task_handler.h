#ifndef __TASK_HANDLER_H__
#define __TASK_HANDLER_H__

#include <common.h>
#include <global.h>

/**
 * @brief Initializes the task descriptor handler
 * @details Initializes the task descriptor handler
 */
void init_task_handler(global_data_t* global_data);

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
 * @param priority The priority to assign to the newly created task. should be a value in the range [0,31]
 * @param code Pointer to the location in memory where the tasks code is located. This is the address that the Task will start executing at when it runs for the first time.
 * 
 * @return
 *  id – the positive integer task id of the newly created task. The task id 
 *      must be unique, in the sense that no task has, will have or has had the
 *      same task id.
 *  -1 – if the priority is invalid.
 *  -2 – if the kernel is out of task descriptors.
 */
int create_task(global_data_t* global_data, priority_t priority, void (*code)());

/**
 * @brief Get a task descriptor for a given tid.
 * @details Get a task descriptor for a given tid.
 * 
 * @param tid The tid of the task to return.
 * @return The task descriptor associated with tid.
 */
task_descriptor_t* get_task(global_data_t* global_data, tid_t tid);

#endif //__TASK_HANDLER_H__
