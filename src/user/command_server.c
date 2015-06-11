#include <command_server.h>
#include <syscalls.h>
#include <servers.h>
#include <common.h>
#include <io.h>

#define USER_INPUT_BUFFER_SIZE 32
static void process_input(char* input);

void command_server(void) {
	RegisterAs(COMMAND_SERVER);
	char byte;
	char input_buffer[USER_INPUT_BUFFER_SIZE];
	char*input_iterator = input_buffer;
	FOREVER {
		byte = Getc(COM2);
		if(input_iterator< (input_buffer+USER_INPUT_BUFFER_SIZE)) {
			if(byte == CARRIAGE_RETURN){
				*input_iterator = '\0'; // terminate the users input with the null char
				//react to the input we currently have
				printf(COM2,"command server got command:\r\n\t%s\r\n",input_buffer);
				process_input(input_buffer);
				//Start inserting chars from the beginning
				input_iterator = input_buffer;
			}else if (byte == BACKSPACE) {
				//TODO: Send server command to backspace
				if(input_iterator>input_buffer) {
					input_iterator--;
				}
			}else{
				*input_iterator = byte;
				input_iterator++;
			}	
			
		}

	}
}

void process_input(char* input) {
	char * argv[10];
	int32_t target_train_number, target_train_value;
	
	int32_t argc = strtokenize(input,argv,10);
	//if(argc < 0); TODO: INPUT TOO LARGE

	//TODO: instance of terminal server command struct
	//char stat_msg[STATUS_SIZE];
	//term_clear_whole_line(t);
	
	//inst->type = NONE;
	if(argc > 3){
		//ERROR too many args
	}else if(argc ==3){
		target_train_number = strtoi(argv[1]);//TODO: add hex support
		if(strcmp(argv[0],"tr")==0){
			target_train_value = strtoi(argv[2]);
			if(((target_train_number) != -1) 
					&& (target_train_value) != -1){
				//tgstrformat(stat_msg,"SET TRAIN %d SPD T0 %d",target_train_number,target_train_value);
				//term_set_status(t,stat_msg);
				//inst->type = TR;
				//inst->target = target_train_number;
				//inst->value = target_train_value;
			}else{
				//term_set_status(t,"ERROR: TR Args invalid");
			}
		}else if(strcmp(argv[0],"sw")==0){
			if(target_train_number >= 0 ){ //TODO: max train number?
				//Set type to SW
				if(strcmp(argv[2],"C")==0){
					//tgstrformat(stat_msg,"SET SWITCH %d TO %s",target_train_number,argv[2]);
					//term_set_status(t,stat_msg);
					
					//	inst->value = 'C';
				}else if(strcmp(argv[2],"S")==0) {
					//	inst->value = 'S';
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
				//tgstrformat(stat_msg,"REVERSE TRAIN %d",target_train_number);
				//term_set_status(t,stat_msg);
				//inst->type = RV;
				//inst->target = target_train_number;
			}else{
				//term_set_status(t,"ERROR: RV, INVALID train number");
			}
		}else{
			//term_set_status(t,"ERROR: Invalid command");
		}
	}else if( argc == 1){

		if(strcmp(argv[0],"q")==0){
			//term_set_status(t,"QUITTING");
  			// /t->operation_state=-1;
		}else{
			//term_set_status(t,"ERROR: Invalid command");
		}
	}else{
		//term_set_status(t,"ERROR: TR Args invalid");
	}
}
