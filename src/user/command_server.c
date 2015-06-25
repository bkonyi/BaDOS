#include <command_server.h>
#include <syscalls.h>
#include <servers.h>
#include <common.h>
#include <io.h>
#include <terminal/terminal.h>
#include <trains/train_controller_commander.h>
#include <trains/track_position_server.h>


#define USER_INPUT_BUFFER_SIZE 32
static void process_input(char* input);

void command_server(void) {
	RegisterAs(COMMAND_SERVER);
	char byte;
	char input_buffer[USER_INPUT_BUFFER_SIZE];
	char*input_iterator = input_buffer; // keep track of the end of the user input
	//terminal_data_t terminal_data;
	FOREVER {
		byte = Getc(COM2);
		if(input_iterator< (input_buffer+USER_INPUT_BUFFER_SIZE)) {
			if(byte == CARRIAGE_RETURN) {
				*input_iterator = '\0'; // terminate the users input with the null char
				//react to the input we currently have
				process_input(input_buffer);
				//Start inserting chars from the beginning
				input_iterator = input_buffer;
			} else if (byte == BACKSPACE) {
				if(input_iterator>input_buffer) {
					input_iterator--;
				}
			} else {
				*input_iterator = byte;
				input_iterator++;
			}	
			
		} else {
			//TODO: ERROR: input too long
		}

	}
}

void process_input(char* input) {
	char * argv[10];
	int32_t result;
	int32_t target_train_number, target_train_value;
	
	int32_t argc = strtokenize(input,argv,10);
	//if(argc < 0); TODO: INPUT TOO LARGE

	terminal_data_t terminal_data;
	//Assume error, prove otherwise
	terminal_data.command = TERMINAL_COMMAND_ERROR;
	
	//inst->type = NONE;
	if(argc > 3) {
		//ERROR too many args
	} else if(argc ==3){
		target_train_number = strtoi(argv[1]);//TODO: add hex support
		if(strcmp(argv[0],"tr")==0) {
			target_train_value = strtoi(argv[2]);
			terminal_data.command = TERMINAL_TRAIN_COMMAND;

			if(((target_train_number) != -1) 
					&& (target_train_value) != -1) {
				terminal_data.num1 = target_train_number;
				terminal_data.num2 = target_train_value;

				result = train_set_speed(target_train_number, target_train_value);

				if(result != 0) {
					terminal_data.command = TERMINAL_COMMAND_ERROR;
				}
			} else {
				//term_set_status(t,"ERROR: TR Args invalid");
			}
		} else if(strcmp(argv[0],"sw")==0) {
			if(target_train_number >= 0 ) { //TODO: max train number?
				//Set type to SW
				terminal_data.command = TERMINAL_SWITCH_COMMAND;
				if((strcmp(argv[2],"C") == 0) || (strcmp(argv[2],"S") == 0) ||
					(strcmp(argv[2], "c") == 0) || (strcmp(argv[2], "s") == 0)) {
					terminal_data.num1 = target_train_number;
					terminal_data.byte1= argv[2][0];

					result = switch_set_direction(target_train_number, argv[2][0]);

					if(result != 0) {
						terminal_data.command = TERMINAL_COMMAND_ERROR;
					}
				} else {
					//term_set_status(t,"ERROR: SW, INVALID switch value");
				}
			} else {
				//term_set_status(t,"ERROR: SW, INVALID switch number");
			}
		} else if(strcmp(argv[0], "register") == 0) {
			terminal_data.command = TERMINAL_REGISTER_TRAIN;
			target_train_number = strtoi(argv[1]);
			int8_t slot = strtoi(argv[2]);

			terminal_data.num1 = target_train_number;
			terminal_data.num2 = slot;

			result = register_train(target_train_number, slot);

			if(result != 0) {
				terminal_data.command = TERMINAL_COMMAND_ERROR;
			}		
		} else {
			//term_set_status(t,"ERROR: Invalid command");
		}
	} else if(argc ==2) {
		
		if(strcmp(argv[0],"rv")==0) {
			target_train_number = strtoi(argv[1]);
			if(target_train_number>0) {
				terminal_data.command = TERMINAL_REVERSE_COMMAND;
				terminal_data.num1 = target_train_number;
				result = train_reverse(target_train_number);

				if(result != 0) {
					terminal_data.command = TERMINAL_COMMAND_ERROR;
				}
			} else {
				printf(COM2,"INVTR %d", target_train_number);
				//term_set_status(t,"ERROR: RV, INVALID train number");
			}
		} else if(strcmp(argv[0], "track") == 0) {
			char track = char_to_upper(argv[1][0]);

			if(track != 'A' && track != 'B') {
				//TODO error
				return;
			} else {
				terminal_data.command = TERMINAL_SET_TRACK;
				terminal_data.byte1 = track;
				if(track == 'A') {
					tps_set_track(TRACKA);
				} else if ( track == 'B' ){
					tps_set_track(TRACKB);
				}
			}
		} else {
			//term_set_status(t,"ERROR: Invalid command");
		}
	} else if( argc == 1) {
		if(strcmp(argv[0],"q")==0){
			terminal_data.command = TERMINAL_QUIT;
		} else if(strcmp(argv[0], "x") == 0) {
			stop_controller();
			terminal_data.command = TERMINAL_STOP_CTRL;
		} else if(strcmp(argv[0], "g") == 0) {
			start_controller();
			terminal_data.command = TERMINAL_START_CTRL;
		} else if(strcmp(argv[0], "find" == 0)) {
			find_trains();
			terminal_data.command = TERMINAL_FIND_TRAIN;
		}
	}
	Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0); //TODO wrap this in terminal.h
}
