#ifndef __CONTEXT_SWITCH_H__
#define __CONTEXT_SWITCH_H__

#include <kernel.h>

void context_switch(task_descriptor_t* td);

#endif