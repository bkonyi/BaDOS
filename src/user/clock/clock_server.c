#include <clock/clock_server.h>
#include <clock/clock_notifier.h>
#include <common.h>
#include <servers.h>
#include <syscalls.h>

void clock_server_task(void) {
    int32_t ticks = 0;
    uint32_t CLOCK_NOTIFIER_TID;
    int32_t receive_tid;
    uint32_t result;
    clock_server_msg_t message;

    RegisterAs(CLOCK_SERVER);

    CLOCK_NOTIFIER_TID = Create(SCHEDULER_HIGHEST_PRIORITY, clock_notifier_task);

    FOREVER {
        Receive(&receive_tid, (char*)(&message), sizeof(clock_server_msg_t));

        //Check to see if the clock notifier has detected a tick.
        if(receive_tid == CLOCK_NOTIFIER_TID) {
            ++ticks;
            Reply(CLOCK_NOTIFIER_TID, (char*)NULL, 0);

            //TODO check to see if we need to respond to any Delay calls here
            continue;
        }

        switch(message.type) {
            case TIME:
                Reply(receive_tid, (char*)&ticks, sizeof(int32_t));
                break;
            case DELAY:
                //TODO
                break;
            case DELAY_UNTIL:
                //TODO
                break;
            default:
                result = -1;
                Reply(receive_tid, (char*)&result, sizeof(int32_t));
                break;
        }
    }
}
