#include <rps/rps_server.h>
#include <syscalls.h>
#include <bwio.h>
#include <servers.h>
#include <common.h>
#include <rps/rps_common.h>
#include <ring_buffer.h>

#define MAX_PLAYERS 500 //Arbitrary

CREATE_NON_POINTER_BUFFER_TYPE(rps_client_queue_t, int, MAX_PLAYERS);

/**
 * @brief Converts a rps_selection to its string format
 * @details Converts a rps_selection to its string format
 * 
 * @param selection The user move enum to convert to a string
 * @return The string name of the user move enum
 */
static char* rps_selection_to_str(rps_selection selection);

/**
 * @brief Plays a game of rock paper scissors between two players.
 * @details Plays a game of rock paper scissors between two players.
 * 
 * @param players The tids of the players.
 * @param moves The moves each player has made.
 * 
 * @return -4 if there is a tie, [-1, -3] if a user quit, the tid of the winning player otherwise
 */
static int play_rps(int* players, rps_msg* moves);

void rps_server_task(void){
	int result;
	int sender_tid;

	rps_msg msg;

	//Queue for waiting players
	rps_client_queue_t waiting_players;
	RING_BUFFER_INIT(waiting_players, MAX_PLAYERS);

	//Queue for current players
	int current_players[2];
	int current_player_count = 0;
	int players_waiting = 0;

	//The moves for each currently playing player
	rps_msg player_moves[2];
	int moves_received = 0;

	int i;
	for(i = 0; i < 2; ++i) {
		player_moves[i].type = RPS_SIGNUP;
		player_moves[i].selection = ROCK;
	}


	//Now that the RPS server is initialized, we should make it available
	RegisterAs(RPS_SERVER);

	FOREVER {

		//Check to see if there are two players waiting for a reply to start playing
		if(players_waiting == 2) {
			//Tell the players it's time to start a game
			result = 0;
			Reply(current_players[0], (char*)&result, sizeof(int));
			Reply(current_players[1], (char*)&result, sizeof(int));
			players_waiting = 0;
		}

		//We should probably only receive when there aren't enough players to play
		Receive(&sender_tid,(char*)&msg, sizeof(rps_msg));

		switch(msg.type){
			case RPS_SIGNUP:
				//If a new player wants to sign up and there's an open slot, add them to the game now
				if(current_player_count < 2) {
					//Add the ready players to the waiting queue
					current_players[current_player_count] = sender_tid;
					++current_player_count;
					++players_waiting;
					bwprintf(COM2, "Player %d has joined the game!\r\n", sender_tid);

				} else { //Otherwise, put them in line
					PUSH_BACK(waiting_players, sender_tid, result);
				}
				break;
			case RPS_WAITING:
				//When a player responds with RPS_WAITING, it means they're a current player ready for the next round
				//and are now waiting for a reply from the server to start the game
				++players_waiting;
				break;
			case RPS_PLAY:
			case RPS_QUIT:

				//Record the moves made by our two current players
				if(sender_tid == current_players[0]) {
					player_moves[0] = msg;
					++moves_received;
				} else if(sender_tid == current_players[1]) {
					player_moves[1] = msg;
					++moves_received;
				}

				//Now that both players have returned their moves, we run the game.
				if(moves_received == 2) {
					int game_result = play_rps(current_players, player_moves);

					//If the game is a tie, return the GAME_TIE code of -4
					if(game_result == GAME_TIE_CODE) {
						//Send the replies to the players
						Reply(current_players[0], (char*)&game_result, sizeof(int));
						Reply(current_players[1], (char*)&game_result, sizeof(int));

					} else if(game_result >= 0) { //Check to see if a player has won
						//Return 1 if a player won, 0 otherwise
						int p1_result =  (game_result == current_players[0]);
						int p2_result = !(game_result == current_players[0]);

						//Send the replies to the players
						Reply(current_players[0], (char*)&p1_result, sizeof(int));
						Reply(current_players[1], (char*)&p2_result, sizeof(int));
					} else {
						//In this case, one or both players are quitting
						result = -1;

						//Send the replies to the players
						Reply(current_players[0], (char*)&result, sizeof(int));
						Reply(current_players[1], (char*)&result, sizeof(int));

						//Reset the player count before we see which player we're keeping around
						current_player_count = 0;

						//Check to see if player 1 is quitting
						if(game_result % 2 == 0) {
							++current_player_count;
						} else {
							bwprintf(COM2, "Player %d is quitting!\r\n", current_players[0]);

							//If there's a waiting player, they can start playing now
							if(BUFFER_LENGTH(waiting_players) != 0) {
								POP_FRONT(waiting_players, current_players[0]);
								bwprintf(COM2, "Player %d has joined the game!\r\n", current_players[0]);
								++players_waiting;
								++current_player_count;
							}
						}

						//Check to see if player 2 is quitting
						if(game_result / 2 == 0) {
							current_players[current_player_count] = current_players[1];
							++current_player_count;
						} else {
							bwprintf(COM2, "Player %d is quitting!\r\n", current_players[1]);

							//If there's a waiting player, they can start playing now
							if(BUFFER_LENGTH(waiting_players) != 0) {
								POP_FRONT(waiting_players, current_players[current_player_count]);
								bwprintf(COM2, "Player %d has joined the game!\r\n", current_players[current_player_count]);
								++players_waiting;
								++current_player_count;
							}
						}
					}

					//Reset the moves received counter
					moves_received = 0;

					bwprintf(COM2, "\r\n");
					bwgetc(COM2);
				}
				break;
			default:
				ASSERT(0);
				break;
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

	if(moves[1].type == RPS_QUIT) {
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
		return GAME_TIE_CODE;
	}

	//Check to see if player 1 won
	if((p1_move == ROCK && p2_move == SCISSORS) ||
		(p1_move == SCISSORS && p2_move == PAPER) ||
		(p1_move == PAPER && p2_move == ROCK)) {
		bwprintf(COM2, "Player %d won!\r\n", players[0]);
		return players[0];
	}

	//Player 2 won
	bwprintf(COM2, "Player %d won!\r\n", players[1]);
	return players[1];
}

