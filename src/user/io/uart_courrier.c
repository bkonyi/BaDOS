#include <io/uart_courrier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>
#include <terminal/terminal.h>

void uart_courrier(void) {
    char byte;
    terminal_data_t echo_command;
    echo_command.command = TERMINAL_ECHO_INPUT;

    RegisterAs(UART2_COURRIER);

    FOREVER {
        Send(UART2_RECEIVE_SERVER_ID, (char*)NULL, 0, &byte, sizeof(char));

        echo_command.byte1 = byte;
        Send(TERMINAL_SERVER_ID, (char*)&echo_command, sizeof(terminal_data_t), (char*)NULL, 0);
    }
}
