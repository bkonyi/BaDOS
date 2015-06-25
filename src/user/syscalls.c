#include <syscalls.h>
#include <request.h>
#include <send_sys_call.h>
#include <bwio.h>
#include <io.h>
#include <servers.h>
#include <clock/clock_server.h>

int Create( int priority, void (*code) () ) {
    return CreateName(priority, code, "UNKNOWN");
}

int CreateName( int priority, void (*code) (), char* name ) {
    request_t request;

    request.sys_code = SYS_CALL_CREATE;
    request.priority = priority;
    request.func     = code;
    request.name     = name;

    return send_sys_call(&request);
}

int Destroy( int tid ) {
    request_t request;

    request.sys_code = SYS_CALL_DESTROY;
    request.receiving_tid = tid;

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

int AwaitEvent( int eventid ) {
    request_t request;

    request.sys_code = SYS_CALL_AWAIT_EVENT;
    request.eventid  = eventid;

    return send_sys_call(&request);
}

int Time( void ) {
    int32_t time_result;
    int32_t result;

    clock_server_msg_t message;
    message.type = TIME;

    result = Send(CLOCK_SERVER_ID, (char*)&message, sizeof(clock_server_msg_t), (char*)&time_result, sizeof(int32_t));
    ASSERT(result == sizeof(int32_t));

    return time_result;
}

int Delay( int ticks ) {
    int32_t result;
    int32_t delay_result = 0;

    clock_server_msg_t message;
    message.type = DELAY;
    message.ticks = ticks;
    
    result = Send(CLOCK_SERVER_ID, (char*)&message, sizeof(clock_server_msg_t), (char*)&delay_result, sizeof(int32_t));

    ASSERT(result == sizeof(int32_t));
    ASSERT(delay_result >= 0);

    return delay_result;
}

int DelayUntil( int ticks ) {
    int32_t result;
    int32_t delay_result = 0;

    clock_server_msg_t message;
    message.type = DELAY_UNTIL;
    message.ticks = ticks;

    result = Send(CLOCK_SERVER_ID, (char*)&message, sizeof(clock_server_msg_t), (char*)&delay_result, sizeof(int32_t));

    ASSERT(result > 0);
    ASSERT(delay_result >= 0);

    return delay_result;
}

//Just for compatability with the kernel spec.
int Getc( int channel ) {
    return getc(channel);
}

//Just for compatability with the kernel spec.
int Putc( int channel, char ch ) {
    return putc(channel, ch);
}

int Terminate(void) {
    request_t request;

    request.sys_code = SYS_CALL_TERMINATE;
    return send_sys_call(&request);
}

int GetIdleTime(void) {
    request_t request;

    request.sys_code = SYS_CALL_GET_IDLE;
    return send_sys_call(&request);
}
