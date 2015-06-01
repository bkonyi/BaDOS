#include <clock/clock_server.h>
#include <clock/clock_notifier.h>
#include <syscalls.h>
#include <common.h>
#include <servers.h>

void clock_server_task(void) {
    uint32_t ticks = 0;
    uint32_t CLOCK_NOTIFIER_TID;
    int32_t receive_tid;
    clock_server_msg_t message;

    RegisterAs(CLOCK_SERVER);

    CLOCK_NOTIFIER_TID = Create(SCHEDULER_HIGHEST_PRIORITY, clock_notifier_task);

    FOREVER {
        //TODO
        Receive(&receive_tid, (char*)(&message), sizeof(clock_server_msg_t));

        if(receive_tid == CLOCK_NOTIFIER_TID) {
            ++ticks;
            Reply(CLOCK_NOTIFIER_TID, (char*)NULL, 0);

            //TODO check to see if we need to respond to any Delay calls here

            continue;
        }

        switch(message.type) {
            case TIME:
                Reply(receive_tid, (char*)&ticks, sizeof(uint32_t));
                break;
            case DELAY:
                //TODO
                break;
            case DELAY_UNTIL:
                //TODO
                break;
            default:
                ASSERT(0);
        }
    }
}
