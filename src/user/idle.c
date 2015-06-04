#include <idle.h>
#include <common.h>
#include <syscalls.h>

void idle_task(void) {
    volatile int a = 0;
    FOREVER {
        //TODO keep track of how long the idle task is running
        ++a; //Just do something to pass the time
    }
}

