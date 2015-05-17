#include <common.h>
#include <bwio.h>
#include <context_switch.h>
#include "kernel.h"
#define FOREVER for(;;)

void switch_context(task_descriptor_t* td) {
    bwprintf(COM2, "Going to context switch... %d\r\n", (unsigned int) td);
    context_switch(td);
    bwprintf(COM2, "Context switch successful!\r\n");
}

static void Test(void) {
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
    init_memory();
    //TODO initialize
    task_descriptor_t* test = GetTD(Create(1, Test+0x218000));

    FOREVER {
        switch_context(test);
    }

    return 0;
}
