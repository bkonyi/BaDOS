
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
int MyTid( void );

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
int MyParentTid( void );

/**
 * @brief cease execution, remaining ready to run.
 * @details Pass causes a task to stop executing. The task is moved to the
 * end of its priority queue, and will resume executing when next scheduled.
 */
void Pass( void );

/**
 * @brief terminate execution forever.
 * @details Exit causes a task to cease execution permanently. It is
 * removed from all priority queues, send queues, receive queues and
 * awaitEvent queues. Resources owned by the task, primarily its memory
 * and task descriptor are not reclaimed.
 */
void Exit( void );

/**
 * @brief sends a message to another task and receives a reply
 * @details sends a message to another task and receives a reply. The
 * message, in a buffer in the sending task’s address space is copied to the
 * address space of the task to which it is sent by the kernel.
 * Send supplies a buffer into which the reply is to be copied, and the size of the buffer so that
 * the kernel can detect overflow. When Send returns without error it is
 * guaranteed that the message has been received, and that a reply has been
 * sent, not necessarily by the same task. If either the message or the reply is a
 * string it is necessary that the length should include the terminating null.
 * 
 * The kernel will not overflow the reply buffer. The caller is expected to
 * compare the return value to the size of the reply buffer. If part of the reply
 * is missing the return value will exceed the size of the supplied reply buffer.
 * There is no guarantee that Send will return. If, for example, the task to
 * which the message is directed never calls Receive, Send never returns and
 * the sending task remains blocked forever.
 * 
 * @param tid The tid of the task the message is to be sent to.
 * @param msg The message to be sent to the task.
 * @param msglen The length of the message to be sent to the task
 * @param reply A buffer in which the reply message will be placed.
 * @param replylen The length of the reply message buffer.
 * @return the size of the message supplied by the replying task.
 *   •-1 – if the task id is impossible.
 *   •-2 – if the task id is not an existing task.
 *   •-3 – if the send-receive-reply transaction is incomplete.
 */
int Send( int tid, char *msg, int msglen, char *reply, int replylen );

/**
 * @brief Wait to receive a message from another task.
 * @details Receive blocks until a message is sent to the caller, then
 * returns with the message in its message buffer and tid set to the task id of
 * the task that sent the message. Messages sent before Receive is called are
 * retained in a send queue, from which they are received in first-come, first-served order.
 * The argument msg must point to a buffer at least as large as msglen. If the size of the message received exceeds
 * msglen, no overflow occurs and the buffer will contain the first msglen characters of the message sent.
 * The caller is expected to compare the return value, which contains the size of the message that was sent, 
 * to determine whether or not the message is complete, and to act accordingly
 * 
 * @param tid The tid of the task which sent the message.
 * @param msg A buffer in which the message will be placed.
 * @param msglen The length of the message buffer.
 * @return The size of the message sent
 */
int Receive( int *tid, char *msg, int msglen );

/**
 * @brief Reply sends a reply to a task that previously sent a message.
 * @details Reply sends a reply to a task that previously sent a message.
 * When it returns without error, the reply has been copied into the sender’s
 * address space. The calling task and the sender return at the same logical
 * time, so whichever is of higher priority runs first. If they are of the same
 * priority the sender runs first.
 * 
 * @param tid The tid of the task which sent the original message.
 * @param reply The reply message to be sent to the task.
 * @param replylen The length of the reply message.
 * @return 
 * • 0 – if the reply succeeds.
 * •-1 – if the task id is not a possible task id.
 * •-2 – if the task id is not an existing task.
 * •-3 – if the task is not reply blocked.
 * •-4 – if there was insufficient space for the entire reply in the sender’s
 * reply buffer.
 */
int Reply( int tid, char *reply, int replylen );

/**
 * @brief Registers the task id of the caller under a given name.
 * @details RegisterAs registers the task id of the caller under the given name.
 * On return without error it is guaranteed that all WhoIs calls by any task
 * will return the task id of the caller until the registration is overwritten.
 * If another task has already registered with the given name its registration is overwritten.
 * A single task may register under several different names, but each name is assigned to a single task.
 * RegisterAs is actually a wrapper covering a send to the name server.
 * 
 * @param name The name to register with the active task's tid
 * @return 
 * • 0 – success.
 * •-1 – if the nameserver task id inside the wrapper is invalid.
 */
int RegisterAs( char *name );

/**
 * @brief Returns the tid associated with a specific task name.
 * @details WhoIs asks the nameserver for the task id of the task that is
 * registered under the given name. Whether WhoIs blocks waiting for a registration or returns with an
 * error if no task is registered under the given name is implementation-dependent.
 * There is guaranteed to be a unique task id associated with each registered name, 
 * but the registered task may change at any time after a call to WhoIs.
 * WhoIs is actually a wrapper covering a send to the nameserver.
 * 
 * @param name The name to lookup.
 * @return 
 *  •tid – the task id of the registered task.
 *  •-1 – if the nameserver task id inside the wrapper is invalid.
 */
int WhoIs( char *name );

/**
 * @brief AwaitEvent blocks until the event identified by eventid occurs
 *   then returns.
 * @details AwaitEvent blocks until the event identified by eventid occurs
 *   then returns. The following details are implementation-dependent.
 *   • Whether or not the kernel collects volatile data and re-enables the
 *   interrupt.
 *   • Whether volatile data is returned as a positive integer in the return
 *   value, or in the event buffer, or not returned at all.
 *   • Whether or not interrupts are enabled when AwaitEvent returns.
 *   • Whether or not to allow more than one task to block on a single
 *   event.
 *
 * 
 * @param eventid The event ID the task is to wait on
 * @returns:
 *   • volatile data – in the form of a positive integer.
 *   • 0 – volatile data is in the event buffer.
 *   •-1 – invalid event.
 *   •-2 – corrupted volatile data. Error indication in the event buffer.
 *   •-3 – volatile data must be collected and interrupts re-enabled in the
 *    caller.
 */
int AwaitEvent( int eventid );

/**
 * @brief Returns the number of ticks since the clock server was created.
 * @details Time returns the number of ticks since the clock server was
 * created and initialized.
 * With a 10 millisecond tick and a 32-bit int there should be neither
 * wraparound nor negative time.
 * 
 * @return The number of ticks since the kernel has started
 */
int Time( void );

/**
 * @brief Delays for a certain number of tasks
 * @details Delay returns after the given number of ticks has elapsed.
 * How long after is not guaranteed because the caller may have to wait on
 * higher priority tasks
 * 
 * @param ticks The number of ticks to delay for.
 * @return 0, -1 on error
 */
int Delay( int ticks );

/**
 * @brief Delays until the clock server has ticked ticks times.
 * @details Delay returns when the time since clock server initialization is
 * greater than the given number of ticks. How long after is not guaranteed
 * because the caller may have to wait on higher priority tasks.
 * 
 * @param ticks The time we want to start executing a delayed task.
 * @return 0, -1 on error
 */
int DelayUntil( int ticks );

/**
 * @brief Get a character from a UART
 * @details Getc returns first unreturned character from the given UART.
 * Getc is actually a wrapper for a send to the serial server.
 * 
 * @param channel The channel to get the character from.
 * @return the returned character on success, -1 otherwise
 */
int Getc( int channel );

/**
 * @brief Send a character from the given UART
 * @details queues the given character for transmission by the given UART.
 * On return, the only guarantee is that the character has been queued.
 * Whether it has been transmistted or received is not guaranteed.
 * Putc is actually a wrapper for a send to the serial server.
 * 
 * @param channel The channel to send the character from
 * @param ch The character to send
 * @return 0 on success, -1 otherwise
 */
int Putc( int channel, char ch );

#endif//__SYS_CALL_H__

