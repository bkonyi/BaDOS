#include <trains/train_server.h>

void train_server(void) {
	int requester;
	char message[10]; 

	FOREVER {
		Receive(&requester,message,sizeof(char)*10);

	}
}
