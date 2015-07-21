#ifndef _TERMINAL_DEBUG_LOG_H_
#define _TERMINAL_DEBUG_LOG_H_
#define DEBUG_LOG_MAX_LEN 80
#define DEBUG_LOG_MAX_DEPTH 24

typedef struct debug_log_t {
	#ifdef DEBUG_LOG
    	char entries[DEBUG_LOG_MAX_DEPTH][DEBUG_LOG_MAX_LEN];
    #else
    	char entries[1][1];
    #endif
    int iterator;
    int size;
    bool border_switch;
}debug_log_t;

void send_term_debug_log_msg(char * message, ...);

void debug_log_init(debug_log_t* debug_log){
	debug_log->size =0;
    debug_log->iterator =0;
    debug_log->border_switch = false;
};
#endif// _TERMINAL_DEBUG_LOG_H_
