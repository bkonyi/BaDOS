#ifndef _TIMER_H_
#define _TIMER_H_
#include "common.h"

/**
 * @brief start the TIMER3 timer at it's maximum value
 */
void time_start();

/**
 * @brief Gets the TIMER3 tick value and produces teh number of microseconds elapsed
 * @return the number of microseconds that have elapsed since time_start.
 */
uint32_t time_stop();

#endif //_TIMER_H_
