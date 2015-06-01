#ifndef __INTERRUPT_HANDLER_H__
#define __INTERRUPT_HANDLER_H__
#include <global.h>

void initialize_interrupts(void);

void handle_interrupt(global_data_t* global_data);

#endif //__INTERRUPT_HANDLER_H__
