#include <msg_sending_tests.h>
#include <syscalls.h>
#include <common.h>
#include <bwio.h>
#include <global.h>

void first_msg_sending_user_task(void) {
    Create(SCHEDULER_LOWEST_PRIORITY+1, receiver);
    Create(SCHEDULER_LOWEST_PRIORITY, sender);
    Exit();
}

void sender(void) {
    int receiver = WhoIs("RECEIVER");
    tid_t my_tid = MyTid();
   
    char* send_buffer = "Hello!";
    size_t send_size = 7;
    char reply_buffer[20];
    size_t reply_buffer_size = 20;
    int result;

    bwprintf(COM2, "Sending message from TID %u to TID %d: %s\r\n", my_tid, receiver, send_buffer);
    result = Send(receiver, send_buffer, send_size, reply_buffer, reply_buffer_size);
    bwprintf(COM2, "Reply on TID %d: %s, Result: %d\r\n", my_tid, reply_buffer, result);

    Exit();
}

void receiver(void) {
    
    RegisterAs("RECEIVER");
    
    tid_t my_tid = MyTid();
    char receive_buffer[20];
    size_t receive_buffer_size = 20;
    int incoming_tid = -1;
    int result;
    
    

    bwprintf(COM2, "Waiting for message on TID %u...\r\n", my_tid);
    result = Receive(&incoming_tid, receive_buffer, receive_buffer_size);

    bwprintf(COM2, "Message Received on TID %u from tid: %d! Message: %s, Result: %d\r\nEchoing message back...\r\n", my_tid, incoming_tid, receive_buffer, result);

    result = Reply(incoming_tid, receive_buffer, result);

    bwprintf(COM2, "Message reply sent from TID %u! Result: %d\r\n", my_tid, result);

    Exit();
}
