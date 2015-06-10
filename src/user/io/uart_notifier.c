#include <io/uart_notifier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>
#include <ts7200.h>
#include <events.h>

void uart_transmit_notifier(uint32_t com) {
    int send_server_tid= -1;
    uint32_t event=0, base=0;

    //Set up our values so that we can properly contact the right UART1 and servers
    switch (com){
        case COM1:
            base = UART1_BASE;
            event = UART1_TRANSMIT_EVENT;
            do {
                send_server_tid = WhoIs(UART1_TRANSMIT_SERVER);
            } while(send_server_tid == -2);
            break;
        case COM2:
            base = UART2_BASE;
            event = UART2_TRANSMIT_EVENT;
            do {
                send_server_tid = WhoIs(UART2_TRANSMIT_SERVER);
            } while(send_server_tid == -2);
            break;
        default: 
            ASSERT(0);
    }
    bwprintf(COM2, "Transmit Notifier created\r\n");
    
    
    int result;

    ASSERT(send_server_tid >= 0);

    //volatile int32_t* UART1_EVENT_REGISTER = (int32_t*)(UART1_BASE + UART_INTR_OFFSET);
    volatile int32_t* UART_DATA_REGISTER  = (int32_t*)(base + UART_DATA_OFFSET);

    FOREVER {
        result = AwaitEvent(event);
        ASSERT(result >= 0);

        //Get the byte to send
        char byte;
        Send(send_server_tid, (char*)NULL, 0, &byte, sizeof(char));

        //Put the byte in the UART register
        *UART_DATA_REGISTER = byte;
    }
}


void uart_receive_notifier(uint32_t com) {
    uint32_t event=0, base=0;
    int receive_server_tid = -1;

    //Set up our values so that we can properly contact the right UART1 and servers
    switch (com){
        case COM1:
            base = UART1_BASE;
            event = UART1_RECEIVE_EVENT;
            do {
                receive_server_tid = WhoIs(UART1_RECEIVE_SERVER);
            } while(receive_server_tid == -2);
            break;
        case COM2:
            base = UART2_BASE;
            event = UART2_RECEIVE_EVENT;
            do {
                receive_server_tid = WhoIs(UART2_RECEIVE_SERVER);
            } while(receive_server_tid == -2);
            break;
        default: 
            ASSERT(0);
    }
   // bwprintf(COM2, "Receive Notifier created\r\n");
    int result;

    ASSERT(receive_server_tid >= 0);

    //volatile int32_t* UART_EVENT_REGISTER = (int32_t*)(base + UART_INTR_OFFSET);
    //volatile int32_t* UART1_DATA_REGISTER  = (int32_t*)(UART1_BASE + UART_DATA_OFFSET);

    FOREVER {
       // bwprintf(COM2,"AWAITING ID: %d\r\n",event);
        result = AwaitEvent(event);
       // bwprintf(COM2,"back from await, eventid: %d\r\n",event);
        ASSERT(result >= 0);

        //Send the byte grabbed to the receive task
        Send(receive_server_tid, (char*)&result, sizeof(char), (char*)NULL, 0);
        
    }
}
void uart1_transmit_notifier(void) {
    uart_transmit_notifier(COM1);
}

void uart1_receive_notifier(void) {
    uart_receive_notifier(COM1);
}
void uart2_transmit_notifier(void) {
    uart_transmit_notifier(COM2);
}

void uart2_receive_notifier(void) {
    uart_receive_notifier(COM2);
}

