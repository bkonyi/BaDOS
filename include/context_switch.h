#ifndef __CONTEXT_SWITCH_H__
#define __CONTEXT_SWITCH_H__

#include <task_handler.h>
#include "request.h"

extern void kerexit(task_descriptor_t* td,request_t* rq);
extern void kerenter();

#endif
