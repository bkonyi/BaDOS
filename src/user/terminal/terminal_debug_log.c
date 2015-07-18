#include <terminal/terminal_debug_log.h>
#include <common.h>
#include <terminal/terminal_types.h>
#include <io/io.h>
#include <servers.h>
#include <trains/train_server_types.h>
void send_term_debug_log_msg(char * message, ...) {

	#ifndef DEBUG_LOG
		return;
	#endif
    char log_entry[DEBUG_LOG_MAX_LEN];
    if(!((strlen(message)+1) < DEBUG_LOG_MAX_LEN)){
        message[DEBUG_LOG_MAX_LEN-1] = '\0';
    }
    va_list va;
    va_start(va,message);
    vsprintf(log_entry,message,va);
    va_end(va);

    int input_len = strlen(log_entry)+1; // include nullus terminus (roman for null terminator....)
    int msg_len = sizeof(terminal_data_t) + input_len*sizeof(char);
    char msg_to_send[msg_len];

    terminal_data_t* terminal_data = (terminal_data_t*)msg_to_send;
    terminal_data->command = TERMINAL_DEBUG_LOG_ENTRY;
    strcpy(msg_to_send + sizeof(terminal_data_t),log_entry);
    Send(TERMINAL_SERVER_ID,msg_to_send,msg_len, NULL,0);
}
