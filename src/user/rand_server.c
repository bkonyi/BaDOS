#include <rand_server.h>
#include <syscalls.h>
#include <common.h>
#include <servers.h>

void rand_server(void) {

    //Initial values chosen randomly by keyboard smash method
    uint32_t x = 123123;
    uint32_t y = 381;
    uint32_t z = 1092;
    uint32_t w = 823238;

    int requester;

    RegisterAs(RAND_SERVER);
    
    FOREVER {

        Receive(&requester, NULL, 0);

        //Xorshift pseudorandom number generator
        //Found on Wikipedia here: http://en.wikipedia.org/wiki/Xorshift

        uint32_t t = x ^ (x << 11);
        x = y; y = z; z = w;
        w = w ^ (w >> 19) ^ t ^ (t >> 8);

        Reply(requester, (void*)(&w), sizeof(uint32_t));
    }

    //We should never get here
    ASSERT(0);
}
