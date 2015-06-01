#include <clock/clock_notifier.h>
#include <syscalls.h>
#include <servers.h>
#include <events.h>
#include <common.h>

void clock_notifier_task(void) {
    uint32_t CLOCK_SERVER_TID;
    uint32_t result;

    CLOCK_SERVER_TID = WhoIs(CLOCK_SERVER);

    FOREVER {
        result = AwaitEvent(TIMER3_EVENT);
        ASSERT(result);

        //Notify the clock server that we've ticked
        Send(CLOCK_SERVER_TID, NULL, 0, NULL, 0);
    }
}
