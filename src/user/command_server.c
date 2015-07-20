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
static int _set_switch(uint32_t train_number,char state);
static int _init_track(char track);
static int _set_speed(uint32_t train_num, uint32_t speed);

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
	char* first = NULL, *second = NULL, *third = NULL, *fourth = NULL;
	
	
	//inst->type = NONE;
	if(argc > 4) {
		send_term_heavy_msg(true,"Too many args");
	} else if(argc == 4) {
		first = argv[0];
		second = argv[1];
		third = argv[2];
		fourth = argv[3];
		if(strcmp(first,"stop_around_sensor") == 0 || strcmp(first, "sas")==0) {
			target_train_number = strtoi(second);
			send_term_cmd_success_msg("stop_around_sensor");
			tcs_send_stop_around_sensor_msg(target_train_number,sensor_to_id(third),strtoi(fourth));
		}
	}else if(argc ==3){
		first = argv[0];
		second = argv[1];
		third = argv[2];

		target_train_number = strtoi(argv[1]);//TODO: add hex support
		if(strcmp(argv[0],"tr")==0) {
			target_train_value = strtoi(argv[2]);

			if(((target_train_number) != -1) 
					&& (target_train_value) != -1) {

				result = _set_speed(target_train_number,target_train_value);

				if(result != 0) {
					send_term_heavy_msg(true,"Error setting train speed");
				}
			} else {
				send_term_heavy_msg(true,"CMD 'TR': invalid args");
				//term_set_status(t,"ERROR: TR Args invalid");
			}
		} else if(strcmp(argv[0],"sw")==0) {
			if(target_train_number >= 0 ) { //TODO: max train number?
				//Set type to SW
				if((strcmp(argv[2],"C") == 0) || (strcmp(argv[2],"S") == 0) ||
					(strcmp(argv[2], "c") == 0) || (strcmp(argv[2], "s") == 0)) {
                    _set_switch(target_train_number, argv[2][0]);
				} else {
					send_term_heavy_msg(true,"CMD SW, INVALID switch value");
				}
			} else {
				send_term_heavy_msg(true,"CMD SW, INVALID switch number");
			}
		} else if(strcmp(argv[0], "register") == 0 || strcmp(argv[0], "reg") == 0) {
			target_train_number = strtoi(argv[1]);
			int8_t slot = strtoi(argv[2]);

			//send_term_register_train_msg(target_train_number,slot);
			result = register_train(target_train_number, slot);
			
			if(result != 0) {
				send_term_heavy_msg(true,"Couldn't register train over train controller");
			}		
		} else if(strcmp(argv[0], "sensor_stop") == 0) {
			target_train_number = strtoi(argv[1]);
			result = trigger_train_stop_on_sensor(target_train_number, sensor_to_id(argv[2]));

			if(result != 0) {
				send_term_heavy_msg(true,"Error telling train controller about stop on sensor");
			}else{
                send_term_cmd_success_msg("sensor_stop");
            }
		} else if (strcmp(argv[0],"track")==0) {
			char track = char_to_upper(argv[1][0]);
				str_to_upper(third);
			if(strcmp(third,"BIGLOOP")==0) {
                _init_track(track);
                _set_switch(6,'S');
                _set_switch(7, 'S');
                _set_switch(8,'S');
                _set_switch(9, 'S');
                _set_switch(12, 'S');
                _set_switch(15, 'S');
                send_term_cmd_success_msg("set track to: bigloop");
			}else if(strcmp(third,"BIGLOOPFLARE")==0) {
                _init_track(track);
                _set_switch(8,'S');
                _set_switch(9, 'S');
                _set_switch(12, 'S');
                _set_switch(15, 'S');
                send_term_cmd_success_msg("set track to: bigloopflare");
			}else if(strcmp(third,"8") == 0||strcmp(third,"8A")==0) {
                _init_track(track);
                _set_switch(10,'S');
                _set_switch(17, 'S');
                _set_switch(156,'S');
                send_term_cmd_success_msg("set track to: 8A");
			} else if(strcmp(third,"8B")==0) {
                _init_track(track);
                _set_switch(13,'S');
                _set_switch(16, 'S');
                _set_switch(154,'S');
                send_term_cmd_success_msg("set track to: 8B");
			}else if (strcmp(third,"SMALLLOOP") == 0) {
				_init_track(track);
                _set_switch(16,'S');
                _set_switch(17, 'S');
                _set_switch(10,'S');
                _set_switch(13,'S');
                send_term_cmd_success_msg("set track to: smallloop");
			}else if (strcmp(third,"MEGALOOP") == 0) {
				_init_track(track);
				if(track == 'B') _set_switch(11,'S');
                _set_switch(4,'S');
                _set_switch(8, 'S');
                _set_switch(9,'S');
                _set_switch(15,'S');
                _set_switch(18, 'S');
                _set_switch(2,'S');
                _set_switch(1,'S');
                send_term_cmd_success_msg("set track to: megaloop");
			}else {
                send_term_heavy_msg(true,"Track configuration '%s' not available",argv[2]);
            }
			
		} else if(strcmp(first,"train_stop_offset") == 0  || strcmp(first, "tso") == 0) {
			target_train_number = strtoi(second);
			send_term_cmd_success_msg("train_stop_offset");
			tcs_send_train_stop_offset_msg(target_train_number,strtoi(third));
		} else if(strcmp(first, "goto") == 0) {
            target_train_number = strtoi(second);
            int8_t sensor_id = sensor_to_id(third);
            
            result = tcs_goto_destination(target_train_number, sensor_id);
            if(result == -1) {
                send_term_heavy_msg(true, "Invalid goto train number");
            } else if(result == -2) {
                send_term_heavy_msg(true, "Invalid goto destination");
            } else {
                send_term_heavy_msg(true, "Sending train: %d to destination: %s", target_train_number, third);
            }
        } else if(strcmp(argv[0], "find" )== 0) {
            target_train_number = strtoi(second);
            int8_t sensor_id = sensor_to_id(third);
            set_train_location(target_train_number, sensor_id);
            send_term_find_msg();
        } else {
			send_term_heavy_msg(true,"Invalid Command");
		}
	} else if(argc ==2) {
		first = argv[0];
		second = argv[1];
		if(strcmp(first,"rv")==0) {
			target_train_number = strtoi(argv[1]);
			if(target_train_number>0) {
				send_term_reverse_msg(target_train_number);
				result = train_reverse(target_train_number);

				if(result != 0) {
					send_term_heavy_msg(true,"Error telling train controller about reverse");
				}
			} else {
				send_term_heavy_msg(true,"RV, invalid train number");
				//term_set_status(t,"ERROR: RV, INVALID train number");
			}
		} else if(strcmp(first, "track") == 0) {
			char track = char_to_upper(argv[1][0]);

			if(track != 'A' && track != 'B') {
				ASSERT(0);
				return;
			} else {
                _init_track(track);
			}
		} else if(strcmp(first, "nudge") == 0) {
			target_train_number = strtoi(second);
			_set_speed(target_train_number,7);
			send_term_heavy_msg(false,"Nudging train %d, please wait",target_train_number);
			Delay(125);
			_set_speed(target_train_number,0);
			send_term_heavy_msg(true,"Done nudging train %d",target_train_number);
		} else if(strcmp(first, "shove") == 0) {
			target_train_number = strtoi(second);
			_set_speed(target_train_number,14);
			send_term_heavy_msg(false,"Shoving train %d, please wait",target_train_number);
			Delay(200);
			_set_speed(target_train_number,0);
			send_term_heavy_msg(true,"Done shoving train %d",target_train_number);
		} else if(strcmp(first, "calibration") == 0 || strcmp(first, "calib") == 0) {
			target_train_number = strtoi(second);
			result = tcs_train_request_calibration_info(target_train_number);

			if(result != 0) {
				send_term_heavy_msg(true, "Invalid train number for calibration info");
			}
		} else if(strcmp(argv[0], "find" )== 0) {
            target_train_number = strtoi(second);
            find_train(target_train_number);
            send_term_find_msg();
        } else {
			send_term_heavy_msg(true,"Invalid command");
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
		} else if(strcmp(argv[0], "tracks" )== 0) {
			
			send_term_heavy_msg(true,"Available track options: smallloop, bigloop, megaloop, 8A, 8B, bigloopflare");
		}  else{
			send_term_heavy_msg(true,"Invalid command");
		}
	}else{
		send_term_heavy_msg(true,"Invalid command");
	}
	
}

int _set_speed(uint32_t train_num, uint32_t speed) {
	int result;
	send_term_train_msg(train_num,speed) ;
	result = tcs_train_set_speed(train_num, speed);
	return result;
}
int _set_switch(uint32_t switch_num,char state) {
    send_term_switch_msg( switch_num,state);
    int result = tcs_switch_set_direction(switch_num, state);
    if(result!=0) {
        send_term_heavy_msg(true,"Error setting switch through train controller");
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
