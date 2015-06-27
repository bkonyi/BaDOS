#include <trains/track_position_server.h>
#include <io.h>
#include <track_data.h>
#include <servers.h>
#include <queue.h>
#include <trains/train_server.h>

#define TPS_SENSOR_MESSAGE_SIZE SENSOR_MESSAGE_SIZE
#define TPS_COVERSHEET_MESSAGE_SIZE (sizeof (tps_cover_sheet_t)) 

#define NUM_SWITCHES_TO_STORE (32+4+4)
#define NUM_MESSAGE_BUFFER_ITEMS 5
#define MESSAGE_BUFFER_SIZE (sizeof(uint32_t)*NUM_MESSAGE_BUFFER_ITEMS)  
#define MAX_NUM_TRAINS 10
static void fill_track_branch_data(track_node* track_nodes, track_node** sensor_track_nodes);
static void tpi_init(train_information_t* train_info, tps_tpi_queue_t* tpi_queue_free, tps_tpi_queue_t* tpi_queue_filled);

static track_node* track_get_sensor(track_node* track_info, uint32_t sensor_number);
static void send_sensor_data_to_trains(tps_tpi_queue_t* train_queue, int8_t* sensors); 
static void notify_trains_switch_changed(tps_tpi_queue_t* train_queue);
static uint32_t switch_num_to_index(uint32_t switch_num);
static bool track_nodes_set_switch(track_node** track_branch_nodes,uint32_t switch_number,uint32_t state);
void track_position_server(void) {
	//The two of these need to be of different sizes
	//we also need to make sure we have enough allocated to hold them
	ASSERT(TPS_SENSOR_MESSAGE_SIZE != TPS_COVERSHEET_MESSAGE_SIZE 
		&& MESSAGE_BUFFER_SIZE > TPS_SENSOR_MESSAGE_SIZE
		&& MESSAGE_BUFFER_SIZE > TPS_COVERSHEET_MESSAGE_SIZE);
	
	int requester, size_received;
	uint32_t message[NUM_MESSAGE_BUFFER_ITEMS];

	//Set up our pointers so we can be sneaky
	//int8_t* signal_message = (int8_t*)message;
	tps_cover_sheet_t* tps_message = (	tps_cover_sheet_t*)message;
	train_information_t* tpip;
	uint32_t state = 0;
	char character;
	track_node* track_branch_nodes[NUM_SWITCHES_TO_STORE]; //merge nodes are attached through the .reverse member
	
	track_node track_nodes[TRACK_MAX];

	tps_tpi_queue_t tpi_queue_filled,tpi_queue_free;

	train_information_t train_info[MAX_NUM_TRAINS];

	tpi_init(train_info,&tpi_queue_free,&tpi_queue_filled);


	FOREVER {
		size_received = Receive(&requester,(char*)message,TPS_SENSOR_MESSAGE_SIZE);

		//printf(COM2,"GOTTI 0x%x",((struct tps_cover_sheet_t*)message)->num1);
		if(! (size_received == TPS_COVERSHEET_MESSAGE_SIZE 
			&& tps_message->command == TPS_ADD_TRAIN ) ){
			//TPS_ADD_TRAIN requires a special  Reply
			// For all others just reply right away
			Reply(requester,NULL,0);
		}

		switch (size_received) {
			case TPS_SENSOR_MESSAGE_SIZE:
				//make sure all trains get the sensor data
				send_sensor_data_to_trains(&tpi_queue_filled,(int8_t*)message);
				break;
			case TPS_COVERSHEET_MESSAGE_SIZE:
				switch(tps_message->command) {
					case TPS_SET_TRACK:
						switch(tps_message->num1) {
							case TRACKA:
								init_tracka(track_nodes);
								break;
							case TRACKB:
								init_trackb(track_nodes);
								break;
							default:
								ASSERT(0);
								break;
						}
						fill_track_branch_data(track_nodes, track_branch_nodes);
						break;
					case TPS_ADD_TRAIN:
						//num1 = train number
						//num2 = switch number
						QUEUE_POP_FRONT(tpi_queue_free,tpip);
						QUEUE_PUSH_BACK(tpi_queue_filled,tpip);
						tpip->train_num = tps_message->num1;
						tpip->server_tid = requester;

						//We want to send back their starting position so they can navigate from there.
						uint32_t literal_addr = (uint32_t)track_get_sensor(track_nodes,tps_message->num2);
						Reply(requester,(char*)&literal_addr,sizeof(uint32_t));
						break;
					case TPS_SWITCH_SET:
						//num1 = switch number
						//num2 = switch state (char)
						character = (char)tps_message->num2 ;
						//bwprintf(COM2,"\r\nTPS Set a switch %d %c\r\n",tps_message->num1,character);
						if (character == 'S' || character == 's') {
							state = DIR_STRAIGHT;
						}else if (character == 'C' || character == 'c') {
							state = DIR_CURVED;
						}else {
							KASSERT(0);
						}

						//If that switch exists then change it and notify the trains that the
							//track has changed
						if(track_nodes_set_switch(track_branch_nodes,tps_message->num1,state)){
							notify_trains_switch_changed(&tpi_queue_filled);
						}
						
						break;
					default:
						break;
				}
				break;
			default:
				bwprintf(COM2,"Invalid Message sent to TPS server\r\n");
				ASSERT(0);
				break;
		}
	}
}

void train_position_info_init(train_position_info_t* tpi) {
	tpi->dist_from_last_sensor = 0;
	tpi->ticks_at_last_sensor = 0;
	tpi->last_sensor = NULL;
}

track_node* tps_add_train(uint32_t train_num) {
	tps_cover_sheet_t tps_message;
	uint32_t track_node_pointer;

	tps_message.command = TPS_ADD_TRAIN;
	tps_message.num1 = train_num;
	tps_message.num2 = 0 ; // TODO: FIND GOES HERE
	Send(TRAIN_POSITION_SERVER_ID,(char*)&tps_message, sizeof(tps_cover_sheet_t),(char*)&track_node_pointer,sizeof(uint32_t));

	return (track_node*)track_node_pointer;
}
void tps_send_sensor_data(int8_t* sensors) {
	//Sensor data has a timestamp tacked on to the end of it so that we can have more 
		//consistent time measurements between trains
	Send(TRAIN_POSITION_SERVER_ID,(char*)sensors, TPS_SENSOR_MESSAGE_SIZE, NULL, 0);
}
void tps_set_track(uint32_t track) {
	//these are the only types of tracks
	ASSERT(track == TRACKA || track == TRACKB);
	tps_cover_sheet_t tps_message;
	tps_message.command = TPS_SET_TRACK;
	tps_message.num1 = track;
	Send(TRAIN_POSITION_SERVER_ID,(char*)&tps_message, sizeof(tps_message),NULL,0);
}
void tps_set_switch(uint32_t sw, char state) {
	tps_cover_sheet_t tps_message;
	tps_message.command = TPS_SWITCH_SET;
	tps_message.num1 = sw;
	tps_message.num2 = (uint32_t)state;
	Send(TRAIN_POSITION_SERVER_ID,(char*)&tps_message, sizeof(tps_message),NULL,0);
}

void fill_track_branch_data(track_node* track_nodes, track_node** track_branch_nodes) {
	int i,branch_index;
	for(i = 0; i < NUM_SWITCHES_TO_STORE; i++) {
		track_branch_nodes[i] = NULL;
	}
	for(i = 0; i < TRACK_MAX; i++) {
		track_nodes[i].state = DIR_STRAIGHT;
		if(track_nodes[i].type == NODE_BRANCH) {

			branch_index = track_nodes[i].num;
			//Make sure to complain if we ever have any switches that are in a range that we haven't handled
			branch_index = switch_num_to_index(branch_index);
			track_branch_nodes[branch_index] = track_nodes+i;
		}
	}
}

void tpi_init(train_information_t* train_info, tps_tpi_queue_t* tpi_queue_free, tps_tpi_queue_t* tpi_queue_filled) {
	QUEUE_INIT(*tpi_queue_filled);
	QUEUE_INIT(*tpi_queue_free);
	int i;
	for(i = 0; i < MAX_NUM_TRAINS; i++) {
		QUEUE_PUSH_BACK(*tpi_queue_free,train_info+i);
	}
}

track_node* track_get_sensor(track_node* track_info, uint32_t sensor_number) {
	int i;
	for(i = 0; i < MAX_NUM_TRAINS; i++) {
		if(track_info[i].type == NODE_SENSOR && track_info[i].num == sensor_number) {
			return track_info+i;
		}
	}
	return NULL;
}

//Returns whether or not that switch exists on this track
bool track_nodes_set_switch(track_node** track_branch_nodes,uint32_t switch_number,uint32_t state) {
	track_node* node = track_branch_nodes[switch_num_to_index(switch_number)];
	if(node != NULL) {
		//printf(COM2,"Setting branch %s to %d\r\n",node->name,(char)state );
		set_track_node_state(node, state);
		return true;
	}
	return false;
}


//Messaging the trains doesn't have a specific struct form yet
void send_sensor_data_to_trains(tps_tpi_queue_t* train_queue, int8_t* sensors) {
	train_information_t* iterator; 
	train_server_sensor_msg_t sensor_update;
	sensor_update.command = TRAIN_SERVER_SENSOR_DATA;

	//TODO make a macro for size of sensor request
	memcpy(sensor_update.sensors, sensors, TPS_SENSOR_MESSAGE_SIZE);
	//bwprintf(COM2,"GEEESH 0x%x\r\n",*((uint32_t*)(sensors+10) ));
	for( iterator = train_queue->head; iterator != NULL; iterator = iterator->next) {
		Send(iterator->server_tid, (char*)&sensor_update, sizeof(train_server_sensor_msg_t), NULL, 0);
	}
}
void notify_trains_switch_changed(tps_tpi_queue_t* train_queue) {
	train_information_t* iterator; 
	for( iterator = train_queue->head; iterator != NULL; iterator = iterator->next) {
		Send(iterator->server_tid,NULL,0,NULL,0);
	}
}
uint32_t switch_num_to_index(uint32_t switch_num) {
	
	ASSERT(is_valid_switch_number(switch_num));
	if(switch_num >= 153) {
		//assume we aren't using indices 28,29,30,31
			//put switches 153,154,155,156 in those spots
		switch_num -= (153-31); 
	}else if (switch_num >= 0x99) {
		switch_num -=(0x99-31+4);
	}
	return switch_num;
}
