#include <syscalls.h>
#include <request.h>
#include <send_sys_call.h>
#include <bwio.h>

int Create( int priority, void (*code) () ) {
    request_t request;

    request.sys_code = SYS_CALL_CREATE;
    request.priority = priority;
    request.func     = code;

    return send_sys_call(&request);
}

int MyTid() {
    request_t request;

    request.sys_code = SYS_CALL_MY_TID;

    return send_sys_call(&request);
}

int MyParentTid() {
    request_t request;

    request.sys_code = SYS_CALL_MY_PARENT_TID;

    return send_sys_call(&request);
}

void Pass() {
    request_t request;

    request.sys_code = SYS_CALL_PASS;

    send_sys_call(&request);
}

void Exit() {
    request_t request;

    request.sys_code = SYS_CALL_EXIT;

    send_sys_call(&request);
}
