#include <io/uart_courrier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>

void uart_courrier(void) {
    int uart_receiver, uart_transmitter;
    char byte;

    do {
        uart_receiver = WhoIs(UART2_RECEIVE_NOTIFIER);
    } while(uart_receiver == -2);

    do {
        uart_transmitter = WhoIs(UART2_TRANSMIT_SERVER);
    } while(uart_transmitter == -2);

    FOREVER {
        Send(uart_receiver, (char*)NULL, 0, &byte, sizeof(char));
        Send(uart_transmitter, &byte, sizeof(char), (char*)NULL, 0);
    }
}
