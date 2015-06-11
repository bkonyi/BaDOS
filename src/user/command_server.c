#include <command_server.h>
#include <syscalls.h>
#include <servers.h>
#include <common.h>
#include <io.h>
#include <terminal/terminal.h>

#define USER_INPUT_BUFFER_SIZE 32
static void process_input(char* input);

void command_server(void) {
	RegisterAs(COMMAND_SERVER);
	char byte;
	char input_buffer[USER_INPUT_BUFFER_SIZE];
	char*input_iterator = input_buffer;
	terminal_data_t terminal_data;
	FOREVER {
		byte = Getc(COM2);
		if(input_iterator< (input_buffer+USER_INPUT_BUFFER_SIZE)) {
			if(byte == CARRIAGE_RETURN){
				*input_iterator = '\0'; // terminate the users input with the null char
				//react to the input we currently have
				process_input(input_buffer);
				//Start inserting chars from the beginning
				input_iterator = input_buffer;
			}else if (byte == BACKSPACE) {
				//TODO: Send server command to backspace
				if(input_iterator>input_buffer) {
					input_iterator--;
					terminal_data.command = TERMINAL_BACKSPACE;
					Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
				}
			}else {
				*input_iterator = byte;
				input_iterator++;
			}	
			
		}else{
			//TODO: ERROR: input too long
		}

	}
}

void process_input(char* input) {
	char * argv[10];
	int32_t target_train_number, target_train_value;
	
	int32_t argc = strtokenize(input,argv,10);
	//if(argc < 0); TODO: INPUT TOO LARGE

	terminal_data_t terminal_data;
	//Assume error, prove otherwise
	terminal_data.command = TERMINAL_COMMAND_ERROR;
	
	//inst->type = NONE;
	if(argc > 3){
		//ERROR too many args
	}else if(argc ==3){
		target_train_number = strtoi(argv[1]);//TODO: add hex support
		if(strcmp(argv[0],"tr")==0){
			target_train_value = strtoi(argv[2]);
			terminal_data.command = TERMINAL_TRAIN_COMMAND;

			if(((target_train_number) != -1) 
					&& (target_train_value) != -1){
				terminal_data.num1 = target_train_number;
				terminal_data.num2 = target_train_value;
			}else{
				//term_set_status(t,"ERROR: TR Args invalid");
			}
		}else if(strcmp(argv[0],"sw")==0){
			if(target_train_number >= 0 ){ //TODO: max train number?
				//Set type to SW
				terminal_data.command = TERMINAL_SWITCH_COMMAND;
				if((strcmp(argv[2],"C")==0) || (strcmp(argv[2],"S")==0)){
					terminal_data.num1 = target_train_number;
					terminal_data.byte1= argv[2][0];
				}else{
					//term_set_status(t,"ERROR: SW, INVALID switch value");
				}
			}else{
				//term_set_status(t,"ERROR: SW, INVALID switch number");
			}
		}else{
			//term_set_status(t,"ERROR: Invalid command");
		}
	}else if(argc ==2){
		
		if(strcmp(argv[0],"rv")==0){
			target_train_number = strtoi(argv[1]);
			if(target_train_number>0){
				terminal_data.command = TERMINAL_REVERSE_COMMAND;
				terminal_data.num1 = target_train_number;
			}else{
				printf(COM2,"INVTR %d", target_train_number);
				//term_set_status(t,"ERROR: RV, INVALID train number");
			}
		}else{
			//term_set_status(t,"ERROR: Invalid command");
		}
	}else if( argc == 1){
		if(strcmp(argv[0],"q")==0){
			terminal_data.command = TERMINAL_QUIT;
		}
	}
	Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
