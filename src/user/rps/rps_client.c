#include <rps/rps_client.h>
#include <syscalls.h>
#include <rps/rps_common.h>
#include <common.h>
#include <servers.h>

static void join_rps(int rps_server);
static void play_rps(int rps_server);
static void quit_rps(int rps_server);

void rps_client_task_3_plays(void) {

    int rps_server_tid;
    do {
        rps_server_tid = WhoIs(RPS_SERVER);
    } while(rps_server_tid == -2);

    join_rps(rps_server_tid);

    int i;
    for(i = 0; i < 3; ++i) {
        play_rps(rps_server_tid);
    }

    quit_rps(rps_server_tid);

    Exit();
}

void rps_client_task_5_plays(void) {

    int rps_server_tid;
    do {
        rps_server_tid = WhoIs(RPS_SERVER);
    } while(rps_server_tid == -2);

    join_rps(rps_server_tid);

    int i;
    for(i = 0; i < 5; ++i) {
        play_rps(rps_server_tid);
    }

    quit_rps(rps_server_tid);

    Exit();
}


void join_rps(int rps_server) {
    rps_msg message;
    message.type = RPS_SIGNUP;

    int result = 0;

    Send((int)rps_server, (char*)&message, sizeof(rps_msg), (char*)&result, sizeof(int));

    ASSERT(result == 0);
}

void play_rps(int rps_server) {
    rps_msg message;
    message.type = RPS_PLAY;
    message.selection = rand() % 3;

    int result;

    //Send our move
    Send((int)rps_server, (char*)&message, sizeof(rps_msg), (char*)&result, sizeof(int));

    //Since we played, we need to wait to start the next game to let the server know if we quit or not
    message.type = RPS_WAITING;
    Send((int)rps_server, (char*)&message, sizeof(rps_msg), (char*)&result, sizeof(int));
}

void quit_rps(int rps_server) {
    rps_msg message;
    message.type = RPS_QUIT;

    int result;
    Send((int)rps_server, (char*)&message, sizeof(rps_msg), (char*)&result, sizeof(int));

    ASSERT(result == -1);
}
