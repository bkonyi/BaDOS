#include <terminal/terminal.h>
#include <common.h>
#include <io/io.h>
#include <servers.h>
#include <syscalls.h>
#include <io/io_common.h>
#include <terminal/terminal_control.h>

#define TERM_INPUT_ROW          3
#define TERM_INPUT_COORDS       8,TERM_INPUT_ROW
#define TERM_STATUS_COORDS      0,2

static void handle_update_terminal_clock(int32_t ticks);
static void terminal_tick_notifier(void);
static void handle_train_command(int32_t num,int32_t speed);
static void handle_reverse_command(int32_t num);
static void handle_switch_command(int32_t num,char state);
static void handle_quit_command(void);
static void clear_user_input(void);
static void status_message(char* fmt, ...);

void terminal_server(void) {
    int sending_tid;
    int result;
    terminal_data_t data;

    RegisterAs(TERMINAL_SERVER);

    term_clear();
    

    
    term_move_cursor(1,1);
    printf(COM2, "Time:");
    term_move_cursor(1,TERM_INPUT_ROW);
    term_green_text();
    printf(COM2, "Input:");
    term_fmt_clr();

    Create(SCHEDULER_HIGHEST_PRIORITY / 2, terminal_tick_notifier);

    //IMPORTANT: This needs to be the last coord we set so user input is in the right place
    term_move_cursor(TERM_INPUT_COORDS);

    FOREVER {
        result = Receive(&sending_tid, (char*)&data, sizeof(terminal_data_t));
        ASSERT(result >= 0);

        //Just send a null reply for now
        Reply(sending_tid, (char*)NULL, 0);

        switch(data.command) {
            case TERMINAL_UPDATE_CLOCK:
                handle_update_terminal_clock(data.num1);
                break;
            case TERMINAL_ECHO_INPUT:
                if(data.byte1 != CARRIAGE_RETURN && data.byte1 != TERMINAL_BACKSPACE) {
                    putc(COM2, data.byte1);
                }
                break;
            case TERMINAL_BACKSPACE:
                //IMPORTANT: Can we guarantee that TERMINAL_BACKSPACE happens right after TERMINAL_ECHO_INPUT for a BACKSPACE? 
                //TODO: Add state to handle that case?
                printf(COM2," %c",BACKSPACE);
                break;
            case TERMINAL_TRAIN_COMMAND:
                handle_train_command(data.num1,data.num2);
                break;
            case TERMINAL_SWITCH_COMMAND:
                handle_switch_command(data.num1,data.byte1);
                break;
            case TERMINAL_REVERSE_COMMAND:
                handle_reverse_command(data.num1);
                break;
            case TERMINAL_QUIT:
                handle_quit_command();
                break;
            case TERMINAL_COMMAND_ERROR:
                status_message("Input Error");
                clear_user_input();
                break;
            default:
                ASSERT(0);
                break;
        }
    }
}

void terminal_tick_notifier(void) {
    int32_t ticks;
    FOREVER {
        ticks = Delay(10);
        update_terminal_clock(ticks); //Delay 100ms
    }
}

void update_terminal_clock(int32_t ticks) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_CLOCK;
    request.num1   = ticks;
    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void handle_update_terminal_clock(int32_t ticks) {
    term_save_cursor();
    term_move_cursor(7,1);
    printf(COM2, "%d:%d.%d", (ticks / 100) / 60, (ticks / 100) % 60, (ticks / 10) % 10);
    term_restore_cursor();
}

/**
 * @brief function that takes the same parameters that printf would take but move the cursor 
 * to the location of the screen where we want our status message to go
 */
void status_message(char* fmt, ...){
    term_move_cursor(TERM_STATUS_COORDS);
    term_clear_rest_line();
    va_list va;
    va_start(va,fmt);
    vprintf(COM2,fmt,va);
    va_end(va);

    clear_user_input();
}
void clear_user_input(void) {
    term_move_cursor(TERM_INPUT_COORDS);
    term_clear_rest_line();
}
void handle_train_command(int32_t num,int32_t speed){
    //
    status_message("CMD: 'TR' #: '%d' Speed: '%d'",num,speed);
}
void handle_reverse_command(int32_t num){
    status_message("CMD: 'RV' #: '%d'",num);
}
void handle_switch_command(int32_t num,char state){
    status_message("CMD 'SW' #: '%d' State: '%c'",num,state);
}
void handle_quit_command(void){
    status_message("CMD QUIT");
    Terminate();
}

