#ifndef __UART1_NOTIFIER_H__
#define __UART1_NOTIFIER_H__
/**
 * Notifiers used to wait on UART interrupts and notify their corresponding servers
 */

void uart1_transmit_notifier(void);
void uart1_receive_notifier(void);

void uart2_transmit_notifier(void);
void uart2_receive_notifier(void);

void uart1_timeout_notifier(void);
void uart2_timeout_notifier(void);

#endif //__UART1_NOTIFIER_H__
