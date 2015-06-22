#include "timers.h"
#include "ts7200.h"



/**
 * @brief Setup the TIMER1 timer for the chip.
 * @details Sets the TIMEMR1 timer with the following values
 * 		- PERIODIC
 * 		- 508 kHz
 * 		- 0xFFFF initial tick value (behaves just like freerun)
 */
static void timer1_init(uint16_t preload) {
    /*
    *TIMER3_LOAD = 508000 * 2;
    *TIMER3_CLEAR = 1;
    *TIMER3_CONTROL |= (0x1 << 6) | (0x1 << 3) | (0x1 << 7);
    */
	uint32_t * line,buf;

    //disable/turnoff timer.
    line = (unsigned int*)TIMER1_CTRL;
    buf = *line;
    buf = buf & ~T1_CTRL_ENABLE_MASK;
    
    //set to 508kHz
    buf = buf | T1_CTRL_CLKSEL_MASK;
   
    //set to periodic 
    buf = buf | T1_CTRL_MODE_MASK;

    //save changes to ctrl
    *line = buf;

    //set the initial time to the given "preload" value
    line = (unsigned int*) TIMER1_LOAD;
    *line = preload;
}

void timer1_start(uint16_t preload) {
	unsigned int * line,buf;
	timer1_init(preload);

	//start/enable timer
    line = (unsigned int*)TIMER1_CTRL;
    buf = *line;
    buf = buf | T1_CTRL_ENABLE_MASK;
    *line = buf;
    buf = *line;
}

void timer1_clear(void) {
    *TIMER1_CLEAR_ADDR = 1;
}

uint32_t timer1_stop(void){
	uint32_t* line, time, buf;
	line = (unsigned int*)TIMER1_VALUE;
    time = *line;
    

    //stop/disable timer
    line = (unsigned int*)TIMER1_CTRL;
    buf = *line;
    buf = buf & ~T1_CTRL_ENABLE_MASK;
    *line = buf;
    buf = *line;
    uint32_t ticks = (TIMER1_MAXCOUNT-time);
    
    //we have 508kHz but want 1000kHz so multiply to account for that
    return ticks * TIMER_508_TO_MICRO;
}

/**
 * @brief Setup the TIMER3 timer for the chip.
 * @details Sets the TIMER3 timer with the following values
 *      - PERIODIC
 *      - 508 kHz
 *      - 0xFFFFFFFF initial tick value (behaves just like freerun)
 */
static void timer3_init(uint32_t preload) {
    /*
    *TIMER3_LOAD = 508000 * 2;
    *TIMER3_CLEAR = 1;
    *TIMER3_CONTROL |= (0x1 << 6) | (0x1 << 3) | (0x1 << 7);
    */
    uint32_t * line,buf;

    //disable/turnoff timer.
    line = (unsigned int*)TIMER3_CTRL;
    buf = *line;
    buf = buf & ~T3_CTRL_ENABLE_MASK;
    
    //set to 508kHz
    //buf = buf | T3_CTRL_CLKSEL_MASK;
   
    //set to periodic 
    buf = buf | T3_CTRL_MODE_MASK;

    //save changes to ctrl
    *line = buf;

    //set the initial time to the given "preload" value
    line = (unsigned int*) TIMER3_LOAD;
    *line = preload;
}

void timer3_freerun(void) {
    timer3_start(TIMER3_MAXCOUNT);
}

void timer3_start(uint32_t preload) {
    unsigned int * line,buf;
    timer3_init(preload);

    //start/enable timer
    line = (unsigned int*)TIMER3_CTRL;
    buf = *line;
    buf = buf | T3_CTRL_ENABLE_MASK;
    *line = buf;
    buf = *line;
}

void timer3_clear(void) {
    *TIMER3_CLEAR_ADDR = 1;
}

uint32_t timer3_get_time(void){
    uint32_t* line, time;
    line = (unsigned int*)TIMER3_VALUE;
    time = *line;

    uint32_t ticks = (TIMER3_MAXCOUNT-time);
    
    //we have 508kHz but want 1000kHz so multiply to account for that
    return ticks * TIMER_508_TO_MICRO;
}

uint32_t timer3_stop(void){
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
