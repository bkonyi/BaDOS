#include <trains/track_position_server.h>
#include <io.h>
#define TPS_SIGNAL_MESSAGE_SIZE (sizeof(int8_t)*10)
#define TPS_COVERSHEET_MESSAGE_SIZE (sizeof (tps_cover_sheet_t)) 

void track_position_server(void) {
	//The two of these need to be of different sizes
	ASSERT(TPS_SIGNAL_MESSAGE_SIZE != TPS_COVERSHEET_MESSAGE_SIZE);
	tps_cover_sheet_t* tps_message;
	int requester, size_received;
	int8_t message[10];

	FOREVER {
		size_received = Receive(&requester,(char *)&message,TPS_SIGNAL_MESSAGE_SIZE);
		
		switch (size_received) {
			case TPS_SIGNAL_MESSAGE_SIZE:
				printf(COM2,"GOT TPS MESSAGE\n");
				break;
			case TPS_COVERSHEET_MESSAGE_SIZE:
				tps_message = (tps_cover_sheet_t*) message;
				break;
			 default:
				printf(COM2,"Invalid Message sent to TPS server\r\n");
				ASSERT(0);
				break;
		}
	}
}

