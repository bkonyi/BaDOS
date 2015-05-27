#include "rps_server.h"
#include "syscalls.h"
#include <bwio.h>
#include <servers.h>
#include <common.h>
#include <rps_common.h>

static char* rps_selection_to_str(rps_selection selection);
static int play_rps(int* players, rps_msg* moves);

void rps_server_task(void){
	int result;
	int sender_tid;

	rps_msg msg;

	//Queue for waiting players
	int players[2];
	int current_players = 0;

	//Now that the RPS server is initialized, we should make it available
	RegisterAs(RPS_SERVER);

	FOREVER {
		//We should probably only receive when there aren't enough players to play
		Receive(&sender_tid,(char*)&msg, sizeof(rps_msg));

		switch(msg.type){
			case RPS_SIGNUP:
				//Add the ready players to the waiting queue
				players[current_players] = sender_tid;
				++current_players;
				bwprintf(COM2, "Player %d has joined the game!\r\n", sender_tid);
				break;
			default:
				result = -1;
				Reply(sender_tid, (char*)&result, sizeof(int));
				break;
		}

		//We play a match as soon as there's 2 players wanting to play
		while(current_players == 2) {
			rps_msg player_moves[2];

			result = 0;

			//Tell the players it's time to start a game
			Reply(players[0], (char*)&result, sizeof(int));
			Reply(players[1], (char*)&result, sizeof(int));

			//Get the responses from each player
			Receive(&players[0], (char*)&player_moves[0], sizeof(rps_msg));
			Receive(&players[1], (char*)&player_moves[1], sizeof(rps_msg));

			int game_result = play_rps(players, player_moves);

			if(game_result < 0) {
				result = -1;

				//Send the replies to the players
				Reply(players[0], (char*)&result, sizeof(int));
				Reply(players[1], (char*)&result, sizeof(int));

				current_players = 0;

				if(game_result % 2 == 0) {
					Receive(&players[0], NULL, 0);
					++current_players;
				} else {
					bwprintf(COM2, "Player %d is quitting!\r\n", players[0]);
				}

				if(game_result / 2 == 0) {
					Receive(&players[current_players], NULL, 0);
					players[current_players] = players[1];
					++current_players;
				} else {
					bwprintf(COM2, "Player %d is quitting!\r\n", players[1]);
				}
			} else {
				int p1_result = (game_result == players[0]);
				int p2_result = (game_result == players[0]);

				//Send the replies to the players
				Reply(players[0], (char*)&p1_result, sizeof(int));
				Reply(players[1], (char*)&p2_result, sizeof(int));
			}

			//Pause after the game is over
			//bwgetc(COM2);

		}
	}
}

char* rps_selection_to_str(rps_selection selection) {
	switch(selection) {
		case ROCK:
			return "ROCK";
		case PAPER:
			return "PAPER";
		case SCISSORS:
			return "SCISSORS";
		default:
			bwprintf(COM2, "UNKNOWN: %d\r\n", selection);
			return "UNKNOWN";
	}

	return "UNKNOWN";
}


int play_rps(int* players, rps_msg* moves) {
	int quitting = 0;

	//Check to see if either player wants to quit
	if(moves[0].type == RPS_QUIT) {
		quitting -= 1;
	}

	if(moves[0].type == RPS_QUIT) {
		quitting -= 2;
	}

	if(quitting != 0) {
		return quitting;
	}

	rps_selection p1_move = moves[0].selection;
	rps_selection p2_move = moves[1].selection;

	bwprintf(COM2, "Player %d played %s and player %d played %s\r\n", 
		players[0], rps_selection_to_str(p1_move), players[1], rps_selection_to_str(p2_move));

	//Check for a tie
	if(p1_move == p2_move) {
		bwprintf(COM2, "There was a tie!\r\n");
		return -4;
	}

	//Check to see if player 1 won
	if((p1_move == ROCK && p2_move == SCISSORS) ||
		(p1_move == SCISSORS && p2_move == PAPER) ||
		(p1_move == PAPER && p2_move == ROCK)) {
		bwprintf(COM2, "Player %d won!\r\n", players[0]);
		return players[0];
	}

	//Player 2 won
	bwprintf(COM2, "Player %d won!\r\n", players[0]);
	return players[1];
}

