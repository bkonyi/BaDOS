#include <idle.h>
#include <common.h>
#include <syscalls.h>
#include <io.h>
#include <bwio.h>

void idle_task(void) {
    volatile int a = 0;
    FOREVER {
        //TODO keep track of how long the idle task is running
        ++a; //Just do something to pass the time

        //TODO remove this testing code
        printf(COM1, "Hello!\r\n");
        printf(COM2, "Test!\r\n");
    }
}

