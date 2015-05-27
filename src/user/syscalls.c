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

int MyTid( void ) {
    request_t request;

    request.sys_code = SYS_CALL_MY_TID;

    return send_sys_call(&request);
}

int MyParentTid( void ) {
    request_t request;

    request.sys_code = SYS_CALL_MY_PARENT_TID;

    return send_sys_call(&request);
}

void Pass( void ) {
    request_t request;

    request.sys_code = SYS_CALL_PASS;

    send_sys_call(&request);
}

void Exit( void ) {
    request_t request;

    request.sys_code = SYS_CALL_EXIT;

    send_sys_call(&request);
}

int Send( int tid, char* msg, int msglen, char* reply, int replylen ) {
    request_t request;

    request.sys_code        = SYS_CALL_SEND;
    request.receiving_tid   = tid;
    request.msg_buffer      = msg;
    request.msg_size        = msglen;
    request.reply_buffer    = reply;
    request.reply_size      = replylen;

    return send_sys_call(&request);
}

int Receive( int* tid, char *msg, int msglen ) {
    request_t request;

    request.sys_code        = SYS_CALL_RECEIVE;
    request.sending_tid     = tid;
    request.msg_buffer      = msg;
    request.msg_size        = msglen;

    return send_sys_call(&request);
}

int Reply( int tid, char* reply, int replylen ) {
    request_t request;

    request.sys_code        = SYS_CALL_REPLY;
    request.receiving_tid   = tid;
    request.reply_buffer    = reply;
    request.reply_size      = replylen;

    return send_sys_call(&request);
}

int RegisterAs( char *name ) {
    nameserver_msg_t t;
    t.tid = MyTid();
    t.name = name;
    t.send_id = REGISTERAS_ID;
    int32_t res;
    Send( NAMESERVER_TID, (char*)&t, sizeof(nameserver_msg_t), (char*)&res, sizeof(int32_t));
    return res; //TODO
}

int WhoIs( char *name ) {
    nameserver_msg_t t;
    t.name = name;
    t.send_id = WHOIS_ID;
    tid_t tid;
    Send(NAMESERVER_TID, (char*)&t, sizeof(nameserver_msg_t),(char*) &tid, sizeof(tid_t));
    return tid; //TODO
}
