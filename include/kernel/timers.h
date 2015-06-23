#ifndef _TIMERS_H_
#define _TIMERS_H_
#include <common.h>

#define TIMER1_LOAD		(TIMER1_BASE + 0x0)
#define TIMER1_VALUE	(TIMER1_BASE + 0x4)	
#define TIMER1_CTRL 	(TIMER1_BASE + 0x8)
    #define T1_CTRL_ENABLE_MASK     0x80 // 1 to enable 0 to disable
    #define T1_CTRL_MODE_MASK       0x40 // 1=>periodic, 0=>freerun
    #define T1_CTRL_CLKSEL_MASK     0x08 // 1=>508kHz, 0=>2kHz
#define TIMER1_CLEAR_ADDR 		((uint32_t*)0x8081000C)

#define TIMER1_MAXCOUNT         (uint32_t)0xFFFF

#define TIMER3_LOAD     (TIMER3_BASE + 0x0)
#define TIMER3_VALUE    (TIMER3_BASE + 0x4) 
#define TIMER3_CTRL     (TIMER3_BASE + 0x8)
    #define T3_CTRL_ENABLE_MASK     0x80 // 1 to enable 0 to disable
    #define T3_CTRL_MODE_MASK       0x40 // 1=>periodic, 0=>freerun
    #define T3_CTRL_CLKSEL_MASK     0x08 // 1=>508kHz, 0=>2kHz
#define TIMER3_CLEAR_ADDR       ((uint32_t*)0x8081008C)

#define TIMER3_MAXCOUNT         (uint32_t)0xFFFFFFFF

#define TIMER_508_TO_MICRO (double)0.508

/**
 * @brief start the TIMER1 timer at it's maximum value
 */
void timer1_start(uint16_t preload);

/**
 * @brief Gets the TIMER1 tick value and produces the number of microseconds elapsed
 * @return the number of microseconds that have elapsed since time_start.
 */
uint32_t timer1_stop(void);

/**
 * @brief Clears the interrupt bit from the timer1 registers
 */
void timer1_clear(void);

/**
 * @brief Starts TIMER3 in freerun mode
 * @details Starts TIMER3 in freerun mode
 */
void timer3_freerun(void);

/**
 * @brief start the TIMER3 timer at it's maximum value
 */
void timer3_start(uint32_t preload);

/**
 * @brief Returns the current tick count on timer 3
 * @details Returns the current tick count on timer 3
 */
uint32_t timer3_get_ticks(void);

/**
 * @brief Gets the TIMER3 tick value and produces the number of microseconds elapsed
 * @return the number of microseconds that have elapsed since time_start.
 */
uint32_t timer3_stop(void);

/**
 * @brief Restarts the timer with current configuration
 * @details Restarts the timer with current configuration
 */
void timer3_restart(void);

/**
 * @brief Clears the interrupt bit from the timer3 registers
 */
void timer3_clear(void);

#endif // _TIMERS_H_
