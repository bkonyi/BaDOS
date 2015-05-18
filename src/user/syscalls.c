#include <syscalls.h>
#include <request.h>

int Create( int priority, void (*code) () ) {
    request_t request;

    request.sys_code = sys_call_create;
    request.priority = priority;
    request.func     = code;

    return send_sys_call(request);
}

int MyTid() {
    request_t request;

    request.sys_code = sys_call_my_tid;

    return send_sys_call(request);
}

int MyParentTid() {
    request_t request;

    request.sys_code = sys_call_my_parent_tid;

    return send_sys_call(request);
}

void Pass() {
    request_t request;

    request.sys_code = sys_call_pass;

    send_sys_call(request);
}

void Exit() {
    request_t request;

    request.sys_code = sys_call_exit;

    send_sys_call(request);
}