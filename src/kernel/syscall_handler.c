#include <syscall_handler.h>
#include <interrupt_handler.h>
#include <scheduler.h>
#include <task_handler.h>
#include <bwio.h>
#include <events.h>
#include <timers.h>
#include <servers.h>

static int handle_Create(global_data_t* global_data, priority_t priority, void (*code)(), char* name) {
    return create_task(global_data, priority, code, name);
}

static int handle_Destroy(global_data_t* global_data, int tid) {
    return destroy_task(global_data, tid);
}

static int handle_MyTid(global_data_t* global_data) {
    task_descriptor_t* td = get_active_task(global_data);
    return td->generational_tid;
} 

static int handle_MyParentTid(global_data_t* global_data) {
    task_descriptor_t* td = get_active_task(global_data);
    return td->parent;
}

static void handle_Pass(global_data_t* global_data) {
    //This is basically a nop used to cause this task to be rescheduled.
    //Nothing to do.
    return;
}

static void handle_Exit(global_data_t* global_data) {
    //zombify_active_task(global_data);
    destroy_task(global_data, get_active_task(global_data)->generational_tid); 
}

static int handle_Send(global_data_t* global_data, int tid, char* msg, int msglen, char* reply, int replylen) {

    task_descriptor_t* active_task = get_active_task(global_data);

    int tid_valid = is_valid_task(global_data, tid);

    //Check to see if the tid belongs to a valid task descriptor
    if(tid_valid) {
        return tid_valid;
    }

    task_descriptor_t* receiving_task = get_task(global_data, tid);

    //Check to see if tid is RECEIVE_BLOCKED.
    //If so, send the message, and become reply blocked
    if(receiving_task->state == TASK_RUNNING_STATE_RECEIVE_BLOCKED) {
        request_t* last_request = receiving_task->last_request;

        int copy_size = min(msglen, last_request->msg_size);

        //Copy the message into the Receiving tasks msg buffer
        memcpy(last_request->msg_buffer, msg, copy_size);

        //Set the sending tid in Receiving tasks address space
        *last_request->sending_tid = active_task->generational_tid;

        //Set the return code to the length of the original message
        receiving_task->return_code = msglen;

        //Reschedule the receive blocked task.
        receiving_task->state = TASK_RUNNING_STATE_READY;
        receiving_task->blocked_on = 0;

        schedule(global_data, receiving_task);
    } else {
        //If not, queue up the message for tid, and become reply blocked
        QUEUE_MESSAGE(receiving_task->message_queue, active_task);
    }

    active_task->state = TASK_RUNNING_STATE_REPLY_BLOCKED;
    active_task->blocked_on = receiving_task->generational_tid;

    //We assume the transaction isn't complete for now
    return -3;
}

static int handle_Receive(global_data_t* global_data, int* tid, char *msg, int msglen) {
    task_descriptor_t* active_task = get_active_task(global_data);

    //Check to see if there are any messages queued up for this task.
    //If so, receive the message, and don't RECEIVE_BLOCK
    if(IS_MESSAGE_WAITING(active_task->message_queue)) {
        task_descriptor_t* sending_task;

        //Get the next queued message
        GET_NEXT_MESSAGE(active_task->message_queue, sending_task);

        request_t* last_request = sending_task->last_request;

        int copy_size = min(msglen, last_request->msg_size);

        //Copy the message into the Receiving tasks msg buffer
        memcpy(msg, last_request->msg_buffer, copy_size);

        //Set the sending tid in the Receiving tasks address space
        *tid = sending_task->generational_tid;

        //Return the actual size of the sent message
        return last_request->msg_size;
    }

    //If not, RECEIVE_BLOCK and wait
    active_task->state = TASK_RUNNING_STATE_RECEIVE_BLOCKED;

    //Set receive size to 0 for now
    return 0;
}

static int handle_Reply(global_data_t* global_data, int tid, char* reply, int replylen) {
    int tid_valid = is_valid_task(global_data, tid);

    if(tid_valid) {
        return tid_valid;
    }

    task_descriptor_t* sending_task = get_task(global_data, tid);

    //Check to see if tid is reply blocked. If not, return an error.
    if(sending_task->state != TASK_RUNNING_STATE_REPLY_BLOCKED) {
        return -3;
    }

    request_t* last_request = sending_task->last_request;

    int copy_size = min(replylen, last_request->reply_size);

    //Check to see if the entry reply can fit within the reply buffer
    if(copy_size < replylen) {
        //TODO should we still perform the partial copy?
        return -4;
    }

    //Copy the reply into the Senders address space
    memcpy(last_request->reply_buffer, reply, copy_size);

    //Set the response length in the return code for Send()
    sending_task->return_code = copy_size;

    //Reschedule the reply blocked task
    sending_task->state = TASK_RUNNING_STATE_READY;
    sending_task->blocked_on = 0;
    schedule(global_data, sending_task);

    return 0;
}

static int handle_AwaitEvent(global_data_t* global_data, int eventid) {
    //Check to see if the event id is valid 
    if(eventid != TIMER1_EVENT && eventid != UART1_TRANSMIT_EVENT &&
        eventid != UART1_RECEIVE_EVENT && eventid != UART2_TRANSMIT_EVENT &&
        eventid != UART2_RECEIVE_EVENT && eventid != UART1_STATUS_EVENT && eventid != UART2_STATUS_EVENT) {
        return -1;
    }

    switch(eventid) {
        case UART1_TRANSMIT_EVENT:
            //Re-enable transmit ready interrupts. Only enabling it here will prevent the
            //interrupt from spamming us when we're not ready for it or don't care about it
            *(volatile uint32_t*)(UART1_BASE + UART_CTLR_OFFSET) |= TIEN_MASK;
            break;
        case UART2_TRANSMIT_EVENT:
            *(volatile uint32_t*)(UART2_BASE + UART_CTLR_OFFSET) |= TIEN_MASK;
            break;
        default:
            break;
    }

    syscall_handler_data_t* syscall_handler_data = &(global_data->syscall_handler_data);
    task_descriptor_t* active_task = get_active_task(global_data);

    active_task->state = TASK_RUNNING_STATE_AWAIT_EVENT_BLOCKED;
    QUEUE_WAITING_TASK(syscall_handler_data->interrupt_waiting_tasks[eventid], active_task);

    //Return -2 for now to represent an incomplete transaction. We'll set this later to a proper value
    return -2;
}

static int handle_GetIdleTime(global_data_t* global_data) {
    return global_data->idle_time_percentage;
}

void initialize_syscall_handler(global_data_t* global_data) {
    syscall_handler_data_t* syscall_handler_data = &(global_data->syscall_handler_data);

    //initialize the queues used for AwaitEvent
    int i;
    for(i = 0; i < NUMBER_OF_EVENTS; ++i) {
        QUEUE_INIT(syscall_handler_data->interrupt_waiting_tasks[i]);
    }
}

void handle(global_data_t* global_data, request_t* request) {
    if(request == NULL) {
        handle_interrupt(global_data);
        return;
    }

    task_descriptor_t* active_task = get_active_task(global_data);

    switch(request->sys_code) {
        case SYS_CALL_CREATE:
            active_task->return_code = handle_Create(global_data, request->priority, request->func, request->name);
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
        case SYS_CALL_SEND:
            active_task->return_code = handle_Send(global_data, request->receiving_tid, request->msg_buffer, 
                request->msg_size, request->reply_buffer, request->reply_size);
            break;
        case SYS_CALL_RECEIVE:
            active_task->return_code = handle_Receive(global_data, request->sending_tid, request->msg_buffer, request->msg_size);
            break;
        case SYS_CALL_REPLY:
            active_task->return_code = handle_Reply(global_data, request->receiving_tid, request->reply_buffer, request->reply_size);
            break;
        case SYS_CALL_AWAIT_EVENT:
            active_task->return_code = handle_AwaitEvent(global_data, request->eventid);
            break;
        case SYS_CALL_GET_IDLE:
            active_task->return_code = handle_GetIdleTime(global_data);
            break;
        case SYS_CALL_DESTROY:
            active_task->return_code = handle_Destroy(global_data, request->receiving_tid);
            break;
        default:
            bwprintf(COM2, "Task: %d Bad system call: %d\r\n", active_task->generational_tid, request->sys_code);
            break;
    }

    active_task->last_request = request;
}
