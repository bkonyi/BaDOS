#ifndef __REQUEST_H__
#define __REQUEST_H__

#include <common.h>

typedef enum {
    SYS_CALL_CREATE        = 0,
    SYS_CALL_MY_TID        = 1,
    SYS_CALL_MY_PARENT_TID = 2,
    SYS_CALL_PASS          = 3,
    SYS_CALL_EXIT          = 4,
    SYS_CALL_SEND          = 5,
    SYS_CALL_RECEIVE       = 6,
    SYS_CALL_REPLY         = 7,
    SYS_CALL_AWAIT_EVENT   = 8,
    SYS_CALL_TERMINATE     = 9
} sys_call_code_t;

typedef struct {
    sys_call_code_t sys_code;

    //Fields for Create()
    uint32_t priority;
    void (*func) ();

    //Fields for Send(), Receive(), Reply()
    char* msg_buffer;
    size_t msg_size;

    char* reply_buffer;
    size_t reply_size;

    int receiving_tid;  //The tid a message is being sent to
    int* sending_tid;   //The tid that sent a message (used for Receive())

    int eventid;        //The event id for AwaitEvent()

} request_t;

#endif // __REQUEST_H__
