#ifndef __CONTEXT_SWITCH_H__
#define __CONTEXT_SWITCH_H__

#include <task_handler.h>
#include <request.h>

/**
 * @brief Enter user task described by td.
 * @details Executes a context switch into the user task.
 * 
 * @param td The description of the user task to execute.
 * @return A pointer to the system call request made by the user task.
 */
extern request_t* kerexit(task_descriptor_t* td);

/**
 * @brief The kernel re-entry point.
 * @details The point where execution returns to the kernel.
 */
extern void kerenter(void);

/**
 * @brief code that handles interrupt requests
 * @details wraps around kerenter to take advantage of the the fact that kerenter
 * is a subset of irq_handler.
 */
extern void irq_handler(void);

#endif
