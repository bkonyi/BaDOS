#include <tests/test_task.h>
#include <common.h>
#include <syscalls.h>
#include <io.h>

void temp(void) {
    //int tid = 0;

    //Receive(&tid, (char*)NULL, 0);
    //ASSERT(Reply(tid, (char*)NULL, 0) >= 0);

    FOREVER {
        Delay(1);
    }
}

void test_task(void) {

	//char c;
	//Delay(500);

	FOREVER {
        int tasks[5];
        tasks[0] = CreateName(6, temp, "TEMP1");
        tasks[1] = CreateName(6, temp, "TEMP2");
        tasks[2] = CreateName(6, temp, "TEMP3");
        tasks[3] = CreateName(6, temp, "TEMP4");
        tasks[4] = CreateName(6, temp, "TEMP5");

        int i;

        for(i = 0; i < 5; ++i) {
            ASSERT(tasks[i] > 0);
        }

        for(i = 0; i < 5; ++i) {
            //ASSERT(Send(tasks[i], (char*)NULL, 0, (char*)NULL, 0) >= 0);
            Delay(10);
            ASSERT(Destroy(tasks[i]) == 0);
        }
	}
}

