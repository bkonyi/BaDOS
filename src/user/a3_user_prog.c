#include <a3_user_prog.h>
#include <bwio.h>
#include <syscalls.h>
#include <common.h>

#define HARDCODED_FIRST_USER_TASK_TID 0

void delay_test_task(void) {

	while(1) {
		Delay(200);
        bwprintf(COM2, "Delay 200\r\n");
	}
}

void delay_test_task2(void) {

	while(1) {
		Delay(150);
        bwprintf(COM2, "Delay 150\r\n");
	}
}

void delay_test_task3(void) {

	while(1) {
		Delay(75);
        bwprintf(COM2, "Delay 75\r\n");
	}
}

void delay_task(void) {
    uint64_t delay_info;
    tid_t my_tid = MyTid();

    Send(HARDCODED_FIRST_USER_TASK_TID, NULL, 0, (char*)&delay_info, sizeof(uint64_t));

    uint32_t delay = delay_info >> (sizeof(uint32_t) * 8);
    uint32_t iterations = (uint32_t)delay_info;

    int delays;

    for(delays = 0; delays < iterations; ++delays) {
        Delay(delay);
        bwprintf(COM2, "TID: %d Interval: %u, Delay Count: %d\r\n", my_tid, delay, delays + 1);
    }

    Exit();
}
