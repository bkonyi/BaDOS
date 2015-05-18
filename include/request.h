#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <common.h>

typedef enum {
    sys_call_create        = 0,
    sys_call_my_tid        = 1,
    sys_call_my_parent_tid = 2,
    sys_call_pass          = 3,
    sys_call_exit          = 4
} sys_call_code_t;

typedef struct {
    sys_call_code_t sys_code;
    uint32_t priority;
    void (*func) ();   
} request_t;

#endif // __REQUEST_H__
