#include <io/uart_servers.h>

#include <bwio.h>

#include <common.h>
#include <nameserver.h>
#include <ring_buffer.h>
#include <servers.h>
#include <syscalls.h>
#include <io/uart1_notifier.h>

#define OUTPUT_BUFFER_SIZE 4096
CREATE_NON_POINTER_BUFFER_TYPE(output_buffer_t, char, OUTPUT_BUFFER_SIZE);

void uart_transmit_server(char* name,uint32_t buffer_size) {
    int sending_tid;
    char byte;
    int result;
    bool output_ready = false;
    int output_notifier = -1;

    output_buffer_t output_buffer;
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
void uart1_transmit_server(void){
    uart_transmit_server(UART1_TRANSMIT_SERVER,OUTPUT_BUFFER_SIZE); 
}
void uart2_transmit_server(void){
    uart_transmit_server(UART2_TRANSMIT_SERVER,OUTPUT_BUFFER_SIZE); 
}

void uart1_receive_server(void) {
    RegisterAs(UART1_RECEIVE_SERVER);

    FOREVER {
        //TODO
    }

    //Shouldn't get here
    Exit();
}
