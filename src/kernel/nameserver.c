#include <nameserver.h>
#include "bwio.h"



/* hash: form hash value for string s */
unsigned hash(global_data_t* global_data, char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % MAX_NAME_SERVER_NAMES;
}

void nameserver_initialize(global_data_t* global_data){
    int i ;
    for(i = 0; i < MAX_NAME_SERVER_NAMES; i++){
        global_data->hashtab[i].filled = false;
    }
}

/* lookup: look for s in hashtab */
tid_t nameserver_lookup(global_data_t* global_data, char *s)
{

    uint32_t i = nameserver_find_slot(global_data,s);
     bwprintf(COM2,"LOOKUP %s\r\n",global_data->hashtab[i].name);
    if(global_data->hashtab[i].filled == true) return global_data->hashtab[i].tid;
    else return -1;
}

uint32_t nameserver_find_slot(global_data_t* global_data, char *name){
    uint32_t i  = hash (global_data,name);
    while(global_data->hashtab[i].filled == true && strcmp(name, global_data->hashtab[i].name) != 0){
        i = (i+1)%MAX_NAME_SERVER_NAMES;
    }
    bwprintf(COM2,"INDEX %d\r\n",i);
    return i;
}

void nameserver_insert(global_data_t* global_data, char * name, tid_t tid){
    uint32_t i = nameserver_find_slot(global_data,name);
    if(global_data->hashtab[i].filled == true){
        //overwrites the old tid for this name
        global_data->hashtab[i].tid = tid;
        return;
    }
    strlcpy(global_data->hashtab[i].name,name,(MAX_TASK_NAME_LENGTH-1));
    global_data->hashtab[i].name[MAX_TASK_NAME_LENGTH-1] = '\0';
    bwprintf(COM2,"STINSERT %s\r\n", global_data->hashtab[i].name);
    global_data->hashtab[i].tid = tid;
    global_data->hashtab[i].filled = true;
}