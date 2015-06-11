#include <io/uart_servers.h>

#include <bwio.h>

#include <common.h>
#include <nameserver.h>
#include <ring_buffer.h>
#include <servers.h>
#include <syscalls.h>
#include <io/uart_notifier.h>
#include <io/uart_courrier.h>

#define OUTPUT_BUFFER_SIZE 4096
#define INPUT_BUFFER_SIZE  2048
CREATE_NON_POINTER_BUFFER_TYPE(uart_buffer_t, char, OUTPUT_BUFFER_SIZE);
CREATE_NON_POINTER_BUFFER_TYPE(uart_tid_buffer_t,tid_t,MAX_NUMBER_OF_TASKS);

void uart_transmit_server(char* name,uint32_t buffer_size) {
    int sending_tid;
    char byte;
    int result;
    bool output_ready = false;
    int output_notifier = -1;

    uart_buffer_t output_buffer;
    RING_BUFFER_INIT(output_buffer, buffer_size);

    RegisterAs(name);

    FOREVER {
        result = Receive(&sending_tid, &byte, sizeof(char));

        //Only the notifier sends messages of length 0
        if(result == 0) {

            //If there's a character on the queue, send it to the notifier
            if(!IS_BUFFER_EMPTY(output_buffer)) {
                POP_FRONT(output_buffer, byte);
                Reply(sending_tid, &byte, sizeof(char));
            } else {
                //Otherwise, we wait to reply until later
                output_ready = true;
                output_notifier = sending_tid;
            }

        } else if(result == sizeof(char)) {

            //Queue up the next character to output
            PUSH_BACK(output_buffer, byte, result);
            ASSERT(result == 0);

            //Let the sender wake back up
            Reply(sending_tid, (char*)NULL, 0);

            //If the UART is waiting on an output, send the byte now.
            if(output_ready) {
                POP_FRONT(output_buffer, byte);
                Reply(output_notifier, &byte, sizeof(char));
                output_ready = false;
            }

        } else {
            //This shouldn't happen
            ASSERT(0);
        }
    }

    //Shouldn't get here
    Exit();
}
void uart1_transmit_server(void) {
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_transmit_notifier);
    uart_transmit_server(UART1_TRANSMIT_SERVER,OUTPUT_BUFFER_SIZE); 
}
void uart2_transmit_server(void) {
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_transmit_notifier);
    uart_transmit_server(UART2_TRANSMIT_SERVER,OUTPUT_BUFFER_SIZE); 
}

void buffered_receive_server(char* name,uint32_t buffer_size) {
    int sending_tid;
    char byte;
    int result;
    tid_t tid;
    
    uart_buffer_t input_buffer;
    uart_tid_buffer_t tid_buffer;
    RING_BUFFER_INIT(input_buffer, buffer_size);
    RING_BUFFER_INIT(tid_buffer, MAX_NUMBER_OF_TASKS);

    RegisterAs(name);
    FOREVER {
        result = Receive(&sending_tid, &byte, sizeof(char));
        
        
        if(result == 0) { //User requests bytes
            PUSH_BACK(tid_buffer, sending_tid, result);
            ASSERT(result == 0);
            //If there's a character on the queue, send it to the notifier
            if(!IS_BUFFER_EMPTY(input_buffer)) {
                POP_FRONT(input_buffer, byte);
                POP_FRONT(tid_buffer,tid);
                Reply(tid, &byte, sizeof(char));
            }
        } else if(result == sizeof(char)) { //Notifier provides byte
            //Queue up the next character to output
            PUSH_BACK(input_buffer, byte, result);
            ASSERT(result == 0);

            //Let the sender wake back up
            Reply(sending_tid, (char*)NULL, 0);

            //If the UART is waiting on an output, send the byte now.
            while(!IS_BUFFER_EMPTY(tid_buffer) && !IS_BUFFER_EMPTY(input_buffer)) {

                POP_FRONT(input_buffer, byte);
                POP_FRONT(tid_buffer,tid);
                Reply(tid, &byte, sizeof(char));
            }
            
        } else {
            //This shouldn't happen
            ASSERT(0);
        }
    }

    //Shouldn't get here
    Exit();
}

void unbuffered_receive_server(char* name,uint32_t buffer_size) {
    int sending_tid;
    char byte;
    int result;
    tid_t tid;
    int courrier_tid;
    bool courrier_ready = false;

    uart_tid_buffer_t tid_buffer;
    RING_BUFFER_INIT(tid_buffer, MAX_NUMBER_OF_TASKS);

    RegisterAs(name);

    do {
        courrier_tid = WhoIs(UART2_COURRIER);
    } while(courrier_tid == -2);

    FOREVER {
        result = Receive(&sending_tid, &byte, sizeof(char));
        
        if(sending_tid == courrier_tid) {
            courrier_ready = true;
            //Nothing more to do here, so go back to the beginning of the loop to receive again
            continue;
        }

        if(result == 0) { //User requests bytes
            PUSH_BACK(tid_buffer, sending_tid, result);
            ASSERT(result == 0);
        } else if(result == sizeof(char)) { //Notifier provides byte
            //Let the sender wake back up
            Reply(sending_tid, (char*)NULL, 0);

            //Check to see if the courrier is ready to echo to screen for COM2
            if(courrier_ready) {
                Reply(courrier_tid, &byte, sizeof(char));
                courrier_ready = false;
            }

            //If the UART is waiting on an output, send the byte now.
            if(!IS_BUFFER_EMPTY(tid_buffer)) {
                POP_FRONT(tid_buffer,tid);
                Reply(tid, &byte, sizeof(char));
            }
        } else {
            //This shouldn't happen
            ASSERT(0);
        }
    }

    //Shouldn't get here
    Exit();
}

void uart1_receive_server(void) {
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart1_receive_notifier);
    buffered_receive_server(UART1_RECEIVE_SERVER,INPUT_BUFFER_SIZE);
}
void uart2_receive_server(void) {
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart2_receive_notifier);
    Create(SCHEDULER_HIGHEST_PRIORITY - 1, uart_courrier);
    unbuffered_receive_server(UART2_RECEIVE_SERVER,INPUT_BUFFER_SIZE);

}
