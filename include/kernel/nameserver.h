#ifndef _NAME_SERVER_H_
#define _NAME_SERVER_H_

#include <global.h>


/**
 * The code for name_server was created using the psuedo code from
 * http://en.wikipedia.org/wiki/Open_addressing
 */


unsigned hash(global_data_t* global_data, char *s);

/* lookup: look for s in hashtab */
tid_t nameserver_lookup(global_data_t* global_data, char *s);

uint32_t nameserver_find_slot(global_data_t* global_data, char *name);

void nameserver_insert(global_data_t* global_data, char* name, tid_t tid);
void nameserver_initialize(global_data_t* global_data);
#endif// _NAME_SERVER_H_
