#ifndef __SYS_CALL_HANDLER_H__
#define __SYS_CALL_HANDLER_H__

#include <global.h>
#include <request.h>

/**
 * @brief System call handler.
 * @details Performs system calls requested by the currently active task and returns results in the
 * current task's task descriptor.
 * 
 * @param global_data [description]
 * @param request The system call request sent by the active task.
 */
void handle(global_data_t* global_data, request_t* request);


#endif //__SYS_CALL_HANDLER_H__
