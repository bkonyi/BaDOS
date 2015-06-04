#include "time.h"
#include "ts7200.h"

#define TIMER3_LOAD		(TIMER3_BASE + 0x0)
#define TIMER3_VALUE	(TIMER3_BASE + 0x4)	
#define TIMER3_CTRL 	(TIMER3_BASE + 0x8)
    #define T3_CTRL_ENABLE_MASK     0x80 // 1 to enable 0 to disable
    #define T3_CTRL_MODE_MASK       0x40 // 1=>periodic, 0=>freerun
    #define T3_CTRL_CLKSEL_MASK     0x08 // 1=>508kHz, 0=>2kHz

#define TIMER_508_TO_MICRO (double)0.508

#define TIMER3_MAXCOUNT         (uint32_t)0xFFFFFFFF

/**
 * @brief Setup the TIMER3 timer for the chip.
 * @details Sets the TIMEMR3 timer with the following values
 * 		- PERIODIC
 * 		- 508 kHz
 * 		- 0xFFFFFFFF initial tick value (behaves just like freerun)
 */
static void time_init() {
	uint32_t * line,buf;
    //disable timer.
    line = (unsigned int*)TIMER3_CTRL;
    buf = *line;
    buf = buf & ~T3_CTRL_ENABLE_MASK;
    
    //set to 508kHz
    buf = buf | T3_CTRL_CLKSEL_MASK;
   
    //set to periodic (technically the same as freerun since we are using the max count for T3)
    buf = buf | T3_CTRL_MODE_MASK;

    //save changes to ctrl
    *line = buf;

    //set the initial time to it's maximum
    line = (unsigned int*) TIMER3_LOAD;
    *line = TIMER3_MAXCOUNT;
}

void time_start() {
	unsigned int * line,buf;
	time_init();

	//start/enable timer
    line = (unsigned int*)TIMER3_CTRL;
    buf = *line;
    buf = buf | T3_CTRL_ENABLE_MASK;
    *line = buf;
    buf = *line;
}

uint32_t time_stop() {
	uint32_t* line, time, buf;
	line = (unsigned int*)TIMER3_VALUE;
    time = *line;
    

    //stop/disable timer
    line = (unsigned int*)TIMER3_CTRL;
    buf = *line;
    buf = buf & ~T3_CTRL_ENABLE_MASK;
    *line = buf;
    buf = *line;
    uint32_t ticks = (TIMER3_MAXCOUNT-time);
    
    //we have 508kHz but want 1000kHz so multiply to account for that
    return ticks * TIMER_508_TO_MICRO;
}
