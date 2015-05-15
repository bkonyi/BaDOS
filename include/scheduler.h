#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <kernel.h>

#define SCHEDULER_NUM_QUEUES            32
#define SCHEDULER_HIGHEST_PRIORITY      0
#define SCHEDULER_LOWEST_PRIORITY       31

/**
 * @brief Initializes the scheduler data structures.
 * @details Initializes the scheduler data structures.
 */
void init_scheduler(void);

/**
 * @brief Schedules a task to be run.
 * @details Schedules a task to be run at a priority specified in the
 *          priority field in task
 * 
 * @param task The task to be scheduled.
 * @return -2 if priority in task is invalid, -1 if task is NULL, 0 on success
 */
int schedule(task_descriptor_t* task);

/**
 * @brief Determines the next task to be run.
 * @details Determines the next task to be run. Reschedules the current active task.
 * @return The new active task to be run.
 */
task_descriptor_t* schedule_next_task(void);

/**
 * @brief Returns the currently active task.
 * @details Returns the currently active task.
 * @return NULL if no task has been scheduled yet, the active task otherwise.
 */
task_descriptor_t* get_active_task(void);

#endif
