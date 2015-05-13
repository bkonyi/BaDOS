#include "common.h"
#include "kernel.h"

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
    next_descriptor->state = TASK_RUNNING_STATE_READY;
    next_descriptor->parent = MyTid();
    next_descriptor->stack = kmalloc(STACK_SIZE);

    //TODO schedule

    return next_tid - 1;
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