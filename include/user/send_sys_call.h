#ifndef __SEND_SYS_CALL_H__
#define __SEND_SYS_CALL_H__

#include <request.h>
/**
 * @brief wrapper function used to call SWI
 * @return The result of the syscall is returned (in R0)
 */
int send_sys_call(request_t* request);

#endif
