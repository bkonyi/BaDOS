#ifndef __UART1_SERVERS_H__
#define __UART1_SERVERS_H__

#define TRANSMIT_BUFFER_SIZE 512 //Max buffer size for sending buffered messages

/**
 * @brief The server which sends data to UART1 
 * @details The server queues up data to be sent to COM1 and sends it when it receives
 * notifications from its notifier
 */
void uart1_transmit_server(void);

/**
 * @brief The server which receives data from UART1
 * @details This server receives data from UART1 and places it into a buffer. When
 *  a task calls getc(COM1), it gets the first message in the buffer.
 */
void uart1_receive_server(void);

/**
 * @brief The server which sends data to UART2 
 * @details The server queues up data to be sent to COM2 and sends it when it receives
 * notifications from its notifier. FIFO is enabled.
 */
void uart2_transmit_server(void);

/**
 * @brief The server which receives data from UART2.
 * @details The server responds with data from UART2 to tasks who have called getc(COM2).
 *  If no tasks are waiting for data from COM2, the data is not buffered and is discarded.
 */
void uart2_receive_server(void);
#endif //__UART1_SERVERS_H__
