#include <syscall_handler.h>
#include <scheduler.h>
#include <task_handler.h>
#include <bwio.h>

int handle_Create(global_data_t* global_data, priority_t priority, void (*code)()) {
    return create_task(global_data, priority, code);
}

int handle_MyTid(global_data_t* global_data) {
    task_descriptor_t* td = get_active_task(global_data);
    return td->tid;
} 

int handle_MyParentTid(global_data_t* global_data) {
    task_descriptor_t* td = get_active_task(global_data);
    return td->parent;
}

void handle_Pass(global_data_t* global_data) {
    //This is basically a nop used to cause this task to be rescheduled.
    //Nothing to do.
    return;
}

void handle_Exit(global_data_t* global_data) {
    zombify_active_task(global_data); 
}

void handle(global_data_t* global_data, request_t* request) {
    task_descriptor_t* active_task = get_active_task(global_data);

    switch(request->sys_code) {
        case SYS_CALL_CREATE:
            active_task->return_code = handle_Create(global_data, request->priority, request->func);
            break;
        case SYS_CALL_MY_TID:
            active_task->return_code = handle_MyTid(global_data);
            break;
        case SYS_CALL_MY_PARENT_TID:
            active_task->return_code = handle_MyParentTid(global_data);
            break;
        case SYS_CALL_PASS:
            handle_Pass(global_data);
            active_task->return_code = 0;
            break;
        case SYS_CALL_EXIT:
            handle_Exit(global_data);
            active_task->return_code = 0;
            break;
        default:
            bwprintf(COM2, "Bad system call: %d\r\n", request->sys_code);
            break;
    }
}
