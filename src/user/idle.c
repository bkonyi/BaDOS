#include <idle.h>
#include <common.h>
#include <syscalls.h>
#include <io.h>
#include <bwio.h>

void idle_task(void) {
    volatile int a = 0;
    FOREVER {
        ++a; //Just do something to pass the time
    }
}

