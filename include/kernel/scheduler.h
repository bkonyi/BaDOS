#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <ring_buffer.h>
#include <task_handler.h>
#include <global.h>

/**
 * @brief Initializes the scheduler data structures.
 * @details Initializes the scheduler data structures.
 */
void init_scheduler(global_data_t* global_data);

/**
 * @brief Schedules a task to be run.
 * @details Schedules a task to be run at a priority specified in the
 *          priority field in task
 * 
 * @param task The task to be scheduled.
 * @return -2 if priority in task is invalid, -1 if task is NULL, 0 on success
 */
int schedule(global_data_t* global_data, task_descriptor_t* task);

/**
 * @brief Determines the next task to be run.
 * @details Determines the next task to be run. Reschedules the current active task.
 * @return The new active task to be run. Returns NULL if there are no more tasks to be 
 * scheduled.
 */
task_descriptor_t* schedule_next_task(global_data_t* global_data);

/**
 * @brief Returns the currently active task.
 * @details Returns the currently active task.
 * @return NULL if no task has been scheduled yet, the active task otherwise.
 */
task_descriptor_t* get_active_task(global_data_t* global_data);

/**
 * @brief Turns the current task into a zombie that won't be scheduled again.
 * @details Turns the current task into a zombie that won't be scheduled again. Does not
 *          release task descriptor to be reused.
 */
void zombify_active_task(global_data_t* global_data);

#endif
