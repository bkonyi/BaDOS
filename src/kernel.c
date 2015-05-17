#include "common.h"
#include "kernel.h"
#include "bwio.h"

#define STACK_SIZE 1024 * 16 //16KiB

static task_descriptor_t tasks[MAX_NUMBER_OF_TASKS];

//Note: we never reassign tids, even if a task is dead.
static tid_t next_tid = 0;

int Create( int priority, void (*code) () ) {
    if(next_tid >= MAX_NUMBER_OF_TASKS) {
        //We're out of task descriptors!
        return -2;
    }

    //TODO check priority here

    task_descriptor_t* next_descriptor = &tasks[next_tid++];
    next_descriptor->state   = TASK_RUNNING_STATE_READY;
    next_descriptor->parent  = MyTid();
    next_descriptor->sp      = (uint32_t) kmalloc(STACK_SIZE) + (sizeof(uint32_t)*20);
    next_descriptor->spsr    = USER_TASK_MODE;
    next_descriptor->pc      = (uint32_t) code;
    ((uint32_t*)next_descriptor->sp)[19]  = (uint32_t) code; // last element sp is the "lr"

    bwprintf(COM2, "Code*: %x PC: %x\r\n", code, next_descriptor->pc);
    
    //TODO schedule

    return next_tid - 1;
}

task_descriptor_t* GetTD(tid_t id) {
    return &tasks[id];
}

int MyTid() {
    //TODO
    return 0;
}

int MyParentTid() {
    return tasks[MyTid()].parent;
}

void Pass() {
    //TODO
}

void Exit() {
    //TODO
}

