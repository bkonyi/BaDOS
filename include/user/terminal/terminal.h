#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <common.h>

typedef enum {
    TERMINAL_UPDATE_CLOCK    =  0,
    TERMINAL_UNKNOWN_COMMAND = -1
} terminal_command_t;

typedef struct {
    terminal_command_t command;
    int32_t            ticks;
} terminal_data_t;

void terminal_server(void);
void update_terminal_clock(int32_t ticks);

#endif //__TERMINAL_H__
