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

    char log_entry[DEBUG_LOG_MAX_LEN*4];

    va_list va;
    va_start(va,message);
    vsprintf(log_entry,message,va);
    va_end(va);

    int input_len = strnlen(log_entry,DEBUG_LOG_MAX_LEN)+1; // include nullus terminus (roman for null terminator....)

    //Best be making sure this ish is Null terminused
    log_entry[input_len-1] = '\0';

    int msg_len = sizeof(terminal_data_t) + input_len*sizeof(char);
    char msg_to_send[msg_len];

    terminal_data_t* terminal_data = (terminal_data_t*)msg_to_send;
    terminal_data->command = TERMINAL_DEBUG_LOG_ENTRY;
    strlcpy(msg_to_send + sizeof(terminal_data_t),log_entry,DEBUG_LOG_MAX_LEN );
    Send(TERMINAL_SERVER_ID,msg_to_send,msg_len, NULL,0);
}
