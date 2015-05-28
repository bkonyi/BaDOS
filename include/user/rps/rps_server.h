#ifndef _RPS_SERVER_H_
#define _RPS_SERVER_H_

/**
 * @brief The Rock-Paper-Scissors Server
 * @details The RPS server accepts and service the following three types of
 *   requests:
 *   • Signup. Signup requests are sent by clients that wish to play. They are
 *   queued when received, and when two are on the queue the server
 *   replies to each, asking for the first choice.
 *   • Play.PlayrequeststelltheserverwhichofRock,PaperorScissorsthe
 *   choose on this round. When play requests have been received from a
 *   pair of clients, the server replies giving the result.
 *   • Quit. Quit tells the server that a client no longer wishes to play. The
 *   server replies to let the client go, and responds to next play request
 *   from the other client by replying that the other player quit.
 */
void rps_server_task(void);

#endif
