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
int _set_switch(uint32_t train_number,char state);
int _init_track(char track);

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

	
	
	//inst->type = NONE;
	if(argc > 3) {
		send_term_error_msg("Too many args");
	} else if(argc ==3){
		target_train_number = strtoi(argv[1]);//TODO: add hex support
		if(strcmp(argv[0],"tr")==0) {
			target_train_value = strtoi(argv[2]);

			if(((target_train_number) != -1) 
					&& (target_train_value) != -1) {

				send_term_train_msg(target_train_number,target_train_value) ;
				result = train_set_speed(target_train_number, target_train_value);

				if(result != 0) {
					send_term_error_msg("Error setting train speed");
				}
			} else {
				send_term_error_msg("CMD 'TR': invalid args");
				//term_set_status(t,"ERROR: TR Args invalid");
			}
		} else if(strcmp(argv[0],"sw")==0) {
			if(target_train_number >= 0 ) { //TODO: max train number?
				//Set type to SW
				if((strcmp(argv[2],"C") == 0) || (strcmp(argv[2],"S") == 0) ||
					(strcmp(argv[2], "c") == 0) || (strcmp(argv[2], "s") == 0)) {
                    _set_switch(target_train_number, argv[2][0]);
				} else {
					send_term_error_msg("CMD SW, INVALID switch value");
				}
			} else {
				send_term_error_msg("CMD SW, INVALID switch number");
			}
		} else if(strcmp(argv[0], "register") == 0 || strcmp(argv[0], "reg") == 0) {
			target_train_number = strtoi(argv[1]);
			int8_t slot = strtoi(argv[2]);

			send_term_register_train_msg(target_train_number,slot);
			result = register_train(target_train_number, slot);
			
			if(result != 0) {
				send_term_error_msg("Couldn't register train over train controller");
			}		
		} else if(strcmp(argv[0], "sensor_stop") == 0) {
			target_train_number = strtoi(argv[1]);
			result = trigger_train_stop_on_sensor(target_train_number, sensor_to_id(argv[2]));

			if(result != 0) {
				send_term_error_msg("Error telling train controller about stop on sensor");
			}
		} else if (strcmp(argv[0],"track")==0) {
			char track = char_to_upper(argv[1][0]);
			if(strcmp(argv[2],"bigloop")==0) {
                _init_track(track);
                _set_switch(6,'S');
                _set_switch(7, 'S');
                _set_switch(8,'S');
                _set_switch(9, 'S');
                _set_switch(12, 'S');
                _set_switch(15, 'S');
			}else {
                send_term_error_msg("Track configuration '%s' not available",argv[2]);
            }
			
		} else {
			send_term_error_msg("Invalid Command");
		}
	} else if(argc ==2) {
		
		if(strcmp(argv[0],"rv")==0) {
			target_train_number = strtoi(argv[1]);
			if(target_train_number>0) {
				send_term_reverse_msg(target_train_number);
				result = train_reverse(target_train_number);

				if(result != 0) {
					send_term_error_msg("Error telling train controller about reverse");
				}
			} else {
				printf(COM2,"INVTR %d", target_train_number);
				send_term_error_msg("");
				//term_set_status(t,"ERROR: RV, INVALID train number");
			}
		} else if(strcmp(argv[0], "track") == 0) {
			char track = char_to_upper(argv[1][0]);

			if(track != 'A' && track != 'B') {
				ASSERT(0);
				return;
			} else {
                _init_track(track);
			}
		} else {
			send_term_error_msg("Invalid command");
		}
	} else if( argc == 1) {
		if(strcmp(argv[0],"q")==0){
			send_term_quit_msg();
		} else if(strcmp(argv[0], "x") == 0) {
			stop_controller();
			send_term_stop_msg();
		} else if(strcmp(argv[0], "g") == 0) {
			start_controller();
			send_term_start_msg();
		} else if(strcmp(argv[0], "find" )== 0) {
			find_trains();
			send_term_find_msg();
		}else{
			send_term_error_msg("Invalid command");
		}
	}else{
		send_term_error_msg("Invalid command");
	}
	
}

int _set_switch(uint32_t train_number,char state) {
    send_term_switch_msg( train_number,state);
    int result = tcs_switch_set_direction(train_number, state);
    if(result!=0) {
        send_term_error_msg("Error setting switch through train controller");
    }
    return result;
}

int _init_track(char track) {
    send_term_set_track_msg(track);
    if(track == 'A') {
        tps_set_track(TRACKA);
    } else if ( track == 'B' ){
        tps_set_track(TRACKB);
    }
    send_term_initialize_track_switches();
    tcs_initialize_track_switches();
    return 0; // Todo: can/should errors be conveyed here? 
}
