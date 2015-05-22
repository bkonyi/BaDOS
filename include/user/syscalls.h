
#ifndef __SYS_CALL_H__
#define __SYS_CALL_H__

/**
 * @brief instantiate a task
 * @details Create allocates and initializes a task descriptor, using the
 * given priority, and the given function pointer as a pointer to the entry
 * point of executable code, essentially a function with no arguments and no
 * return value. When Create returns the task descriptor has all the state
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
int Create( int priority, void (*code) () );

/**
 * @brief find my task id.
 * @return 
 * tid – the positive integer task id of the task that calls it.
 * Errors should be impossible!
 */
int MyTid();

/**
 * @brief find the task id of the task that created the running
task.
 * @details
 * The parent id is stored as an interger in the TD of the calling task. There is no guarantee 
 * that the parent task is still running or is in fact the same task
 * @return 
 * tid – the task id of the task that created the calling task.
 * The original parent id is returned, whether or not that task has finished running or not.
 * Since resource recycling hasn't been implemented yet, then this is guaranteed NOT to be 
 * associated to any new task that is created
 */
int MyParentTid();

/**
 * @brief cease execution, remaining ready to run.
 * @details Pass causes a task to stop executing. The task is moved to the
 * end of its priority queue, and will resume executing when next scheduled.
 */
void Pass();

/**
 * @brief terminate execution forever.
 * @details Exit causes a task to cease execution permanently. It is
 * removed from all priority queues, send queues, receive queues and
 * awaitEvent queues. Resources owned by the task, primarily its memory
 * and task descriptor are not reclaimed.
 */
void Exit();

#endif//__SYS_CALL_H__

