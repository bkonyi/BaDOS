#include <io/uart1_notifier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>
#include <ts7200.h>
#include <events.h>

void uart1_transmit_notifier(void) {
    
    int send_server_tid = WhoIs(UART1_TRANSMIT_SERVER);
    int result;

    ASSERT(send_server_tid >= 0);

    volatile int32_t* UART1_EVENT_REGISTER = (int32_t*)(UART1_BASE + UART_INTR_OFFSET);
    volatile int32_t* UART1_DATA_REGISTER  = (int32_t*)(UART1_BASE + UART_DATA_OFFSET);

    FOREVER {
        result = AwaitEvent(UART1_TRANSMIT_EVENT);
        ASSERT(result >= 0);

        if(*UART1_EVENT_REGISTER & TIS_MASK) {
            //Check to see if Transmit Interrupt Status is set

            //Disable transmit ready interrupt
            //TODO

            //Get the byte to send
            char byte;
            Send(send_server_tid, (char*)NULL, 0, &byte, sizeof(char));

            //Put the byte in the UART register
            *UART1_DATA_REGISTER = byte;

            //Re-enable transmit ready interrupt
            //TODO
        }

    }
}

void uart1_receive_notifier(void) {
    
    int receive_server_tid = WhoIs(UART1_RECEIVE_SERVER);
    int result;

    ASSERT(receive_server_tid >= 0);

    volatile int32_t* UART1_EVENT_REGISTER = (int32_t*)(UART1_BASE + UART_INTR_OFFSET);
    //volatile int32_t* UART1_DATA_REGISTER  = (int32_t*)(UART1_BASE + UART_DATA_OFFSET);

    FOREVER {
        result = AwaitEvent(UART1_RECEIVE_EVENT);
        ASSERT(result >= 0);

        if(*UART1_EVENT_REGISTER & RIS_MASK) {
            //Check to see if Receive Interrupt Status is set

            //Send the byte grabbed to the receive task
            Send(receive_server_tid, (char*)&result, sizeof(char), (char*)NULL, 0);
        }
    }
}
