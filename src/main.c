#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include "kernel.h"
#define FOREVER for(;;)

void switch_context(task_descriptor_t* td) {
    bwprintf(COM2, "Going to context switch...\r\n");
    context_switch(td);
    bwprintf(COM2, "Context switch successful!\r\n");
}

void Test(void) {
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    bwprintf(COM2, "In Test!\r\n");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    __asm__("nop\n\t");
	return;
}

int main(void)
{
    //TODO initialize
    task_descriptor_t* test = GetTD(Create(1, Test));

    FOREVER {
        switch_context(test);
    }

    return 0;
}
