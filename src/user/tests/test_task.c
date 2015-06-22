#include <tests/test_task.h>
#include <common.h>
#include <syscalls.h>
#include <io.h>
void test_task(void) {

	//char c;
	//Delay(500);

	FOREVER {
        int idle_percentage = GetIdleTime();
        printf(COM2, "Idle Time: %d\r\n", idle_percentage);

        int i;
        for(i = 0; i < 10000; ++i) {
            Time();
        }

        Delay(100);
	}
}
