#ifndef _TIMER3_H_
#define _TIMER3_H_
#include <common.h>

#define TIMER3_LOAD		(TIMER3_BASE + 0x0)
#define TIMER3_VALUE	(TIMER3_BASE + 0x4)	
#define TIMER3_CTRL 	(TIMER3_BASE + 0x8)
    #define T3_CTRL_ENABLE_MASK     0x80 // 1 to enable 0 to disable
    #define T3_CTRL_MODE_MASK       0x40 // 1=>periodic, 0=>freerun
    #define T3_CTRL_CLKSEL_MASK     0x08 // 1=>508kHz, 0=>2kHz
#define TIMER3_CLEAR_ADDR 		((uint32_t*)0x8081008C)
#define TIMER_508_TO_MICRO (double)0.508

#define TIMER3_MAXCOUNT         (uint32_t)0xFFFFFFFF

/**
 * @brief start the TIMER3 timer at it's maximum value
 */
void timer3_start(uint32_t preload);

/**
 * @brief Gets the TIMER3 tick value and produces the number of microseconds elapsed
 * @return the number of microseconds that have elapsed since time_start.
 */
uint32_t timer3_stop();
/**
 * @brief Clears the interrupt bit from the timer3 registers
 */
void timer3_clear();

#endif // _TIMER3_H_
