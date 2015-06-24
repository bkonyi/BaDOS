#include <trains/track_position_server.h>
#include <io.h>
#include <track_data.h>
#include <servers.h>

#define TPS_SIGNAL_MESSAGE_SIZE (sizeof(int8_t)*10)
#define TPS_COVERSHEET_MESSAGE_SIZE (sizeof (tps_cover_sheet_t)) 

#define NUM_SWITCHES_TO_STORE 32
void fill_track_branch_data(track_node* track_nodes, track_node** sensor_track_nodes);

void track_position_server(void) {
	//The two of these need to be of different sizes
	ASSERT(TPS_SIGNAL_MESSAGE_SIZE != TPS_COVERSHEET_MESSAGE_SIZE && TPS_SIGNAL_MESSAGE_SIZE > TPS_COVERSHEET_MESSAGE_SIZE);
	
	int requester, size_received;
	char message[30];
	int8_t* signal_message = (int8_t*)message;
	struct tps_cover_sheet_t* tps_message = (struct tps_cover_sheet_t*)message;

	bwprintf(COM2,"BEEEEFFFFF");
	tps_message->num1 = 3; // THIS IS WHERE IT CRAPS OUT
	bwprintf(COM2,"AAFFFFFTTTT");
	//track_node* track_branch_nodes[NUM_SWITCHES_TO_STORE]; //merge nodes are attached through the .reverse member
	//track_node track_nodes[TRACK_MAX];

	FOREVER {
		size_received = Receive(&requester,message,TPS_SIGNAL_MESSAGE_SIZE);
		//printf(COM2,"GOTTI 0x%x",((struct tps_cover_sheet_t*)message)->num1);
		Reply(requester,NULL,0);

		
		switch (size_received) {
			case TPS_SIGNAL_MESSAGE_SIZE:
				printf(COM2,"GOT TPS MESSAGE\n");
				(void)signal_message;
				break;
			case TPS_COVERSHEET_MESSAGE_SIZE:
			(void)tps_message;
			//tps_message->num1 = 2;
			printf(COM2, "EHEHEHEHEHEHHE addr 0x%x 0x%x 0x%x",(uint32_t)message,(uint32_t)tps_message,(uint32_t)signal_message);
				/*switch(tps_message->command) {
					case TPS_SET_TRACK:
						//printf(COM2, "track 0x%x",'D');
						
						switch(TRACKA) {
							case TRACKA:
								//printf(COM2, "                                                                     SETTING TRACKAAAAAAAAAAAAA");
								
								init_tracka(track_nodes);
								break;
							case TRACKB:
								init_trackb(track_nodes);
								break;
							default:
								//KASSERT(0);
								break;
						}
						fill_track_branch_data(track_nodes, track_branch_nodes);
						break;
					default:
						break;
				}*/
				break;
			default:
				//printf(COM2,"Invalid Message sent to TPS server\r\n");
				//ASSERT(0);
				break;
		}
	}
}

void tps_set_track(uint32_t track) {
	//these are the only types of tracks
	ASSERT(track == TRACKA || track == TRACKB);
	tps_cover_sheet_t tps_message;
	tps_message.command = TPS_SET_TRACK;
	tps_message.num1 = track;
	Send(TRAIN_POSITION_SERVER_ID,(char*)&tps_message, sizeof(tps_message),NULL,0);
}

void fill_track_branch_data(track_node* track_nodes, track_node** track_branch_nodes) {
	int i,branch_index;
	for(i = 0; i < TRACK_MAX; i++) {
		track_branch_nodes[i] = NULL;
		if(track_nodes[i].type == NODE_BRANCH) {

			branch_index = track_nodes[i].num;
			//Make sure to complain if we ever have any switches that are in a range that we haven't handled
			ASSERT(!((branch_index>=(NUM_SWITCHES_TO_STORE -4 -1/* -1 for indexed val */)
									 && branch_index < 153) 
					|| branch_index > 156));

			if(branch_index >= 153) {
				//assume we aren't using indices 28,29,30,31
					//put switches 153,154,155,156 in those spots
				branch_index -= (156-31); 
			}
			track_branch_nodes[branch_index] = track_branch_nodes[branch_index];
		}
	}
}

