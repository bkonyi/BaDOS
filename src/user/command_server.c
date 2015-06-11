#include <command_server.h>
#include <syscalls.h>
#include <servers.h>
#include <common.h>
#include <io.h>
void command_server(void) {
	RegisterAs(COMMAND_SERVER);
	char byte;
	FOREVER {
		byte = Getc(COM2);
		printf(COM2,"CS: got char %c\r\n",byte);

	}
}

void process_input(char* input) {
	char* buff = t->uibuff;
	char * argv[10];
	int target_number,target_value;
	
	int argc = term_userinput_getargs(buff,argv);
	char stat_msg[STATUS_SIZE];
	term_clear_whole_line(t);
	
	inst->type = NONE;
	if(argc > 3){
		//ERROR too many args
	}else if(argc ==3){
		if(tgstreq(argv[0],"tr")==0){
			
      target_number = strtoi(argv[1]);
      target_value = strtoi(argv[2]);
			if(((target_number) != -1) 
					&& (target_value) != -1){
				tgstrformat(stat_msg,"SET TRAIN %d SPD T0 %d",target_number,target_value);
				term_set_status(t,stat_msg);
				inst->type = TR;
				inst->target = target_number;
				inst->value = target_value;
			}else{
				term_set_status(t,"ERROR: TR Args invalid");
			}
		}else if(tgstreq(argv[0],"sw")==0){
			if(((target_number = strtoi(argv[1])) != -1) ){
				if((tgstreq(argv[2],"C")==0)
						||tgstreq(argv[2],"S")==0){
					tgstrformat(stat_msg,"SET SWITCH %d TO %s",target_number,argv[2]);
					term_set_status(t,stat_msg);
					inst->type = SW;
					inst->target = target_number;
					if(argv[2][0]=='C'){
						inst->value = 'C';
					}else{
						inst->value = 'S';
					}
					
				}else{
					term_set_status(t,"ERROR: SW, INVALID switch value");
				}
			}else{
				term_set_status(t,"ERROR: SW, INVALID switch number");
			}
		}else{
			term_set_status(t,"ERROR: Invalid command");
		}
	}else if(argc ==2){
		
		if(tgstreq(argv[0],"rv")==0){
			target_number = strtoi(argv[1]);
			if(target_number>0){
				tgstrformat(stat_msg,"REVERSE TRAIN %d",target_number);
				term_set_status(t,stat_msg);
				inst->type = RV;
				inst->target = target_number;
			}else{
				term_set_status(t,"ERROR: RV, INVALID train number");
			}
		}else{
			term_set_status(t,"ERROR: Invalid command");
		}
	}else if( argc == 1){

		if(tgstreq(argv[0],"q")==0){
			term_set_status(t,"QUITTING");
  		t->operation_state=-1;
		}else{
			term_set_status(t,"ERROR: Invalid command");
		}
	}else{
		term_set_status(t,"ERROR: TR Args invalid");
	}
}