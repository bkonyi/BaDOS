#include <a3_user_prog.h>
#include <bwio.h>
#include <syscalls.h>
void delay_test_task(void){

	while(1){
		Delay(200);
	}
}
void delay_test_task2(void){

	while(1){
		Delay(150);
	}
}
void delay_test_task3(void){

	while(1){
		Delay(75);
	}
}
