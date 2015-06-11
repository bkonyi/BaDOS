#include <terminal/terminal.h>
#include <common.h>
#include <io/io.h>
#include <servers.h>
#include <syscalls.h>

static void handle_update_terminal_clock(int32_t ticks);
static void terminal_tick_notifier(void);

void terminal_server(void) {
    int sending_tid;
    int result;
    terminal_data_t data;

    RegisterAs(TERMINAL_SERVER);

    printf(COM2, "\x1B[2J\x1B[1;1H");
    printf(COM2, "Time:\r\n");

    Create(SCHEDULER_HIGHEST_PRIORITY / 2, terminal_tick_notifier);

    FOREVER {
        result = Receive(&sending_tid, (char*)&data, sizeof(terminal_data_t));
        ASSERT(result >= 0);

        //Just send a null reply for now
        Reply(sending_tid, (char*)NULL, 0);

        switch(data.command) {
            case TERMINAL_UPDATE_CLOCK:
                handle_update_terminal_clock(data.ticks);
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
    request.ticks   = ticks;
    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void handle_update_terminal_clock(int32_t ticks) {
    printf(COM2, "\x1B[s\x1B[1;7H");
    printf(COM2, "%d:%d.%d", (ticks / 100) / 60, (ticks / 100) % 60, (ticks / 10) % 10);
    printf(COM2, "\x1B[u");
}
