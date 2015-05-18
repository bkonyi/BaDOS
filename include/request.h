#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <common.h>

typedef enum {
    SYS_CALL_CREATE        = 0,
    SYS_CALL_MY_TID        = 1,
    SYS_CALL_MY_PARENT_TID = 2,
    SYS_CALL_PASS          = 3,
    SYS_CALL_EXIT          = 4
} sys_call_code_t;

typedef struct {
    sys_call_code_t sys_code;
    uint32_t priority;
    void (*func) ();
} request_t;

#endif // __REQUEST_H__
