#ifndef __UART1_SERVERS_H__
#define __UART1_SERVERS_H__

#define TRANSMIT_BUFFER_SIZE 512

void uart1_transmit_server(void);

void uart1_receive_server(void);

void uart2_transmit_server(void);
void uart2_receive_server(void);
#endif //__UART1_SERVERS_H__
