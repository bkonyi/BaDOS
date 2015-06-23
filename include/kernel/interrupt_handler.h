#ifndef __INTERRUPT_HANDLER_H__
#define __INTERRUPT_HANDLER_H__
#include <global.h>

void cleanup_interrupts(void);

void initialize_interrupts(global_data_t* global_data);

/**
 * @brief handles any interrupts that might have been set
 * @details when calling this function, there should be at least 
 * 1 interrupt set
 * 
 * @param global_data the global data structure, used to hold global
 * state
 */
void handle_interrupt(global_data_t* global_data);

#endif //__INTERRUPT_HANDLER_H__
