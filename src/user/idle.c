#include <idle.h>
#include <common.h>
#include <syscalls.h>

void idle_task(void) {
    FOREVER {
        //TODO keep track of how long the idle task is running
        Pass();
        //bwprintf(COM2,"Time time time: %d\r\n",666);
    }
}

