#include <nameserver.h>
#include <common.h>
#include "bwio.h"

#define MAX_TASK_NAME_LENGTH  20
#define MAX_NAME_SERVER_NAMES 200

typedef struct { /* table entry: */
    char name[MAX_TASK_NAME_LENGTH]; /* defined name */
    tid_t tid;
    bool filled; /* replacement text */
} nameserver_list_t;


//Forward declarations of nameserver hash table functions
static uint32_t hash(char *s);
static void nameserver_initialize(nameserver_list_t* hashtab, size_t hashtab_size);
static tid_t nameserver_lookup(nameserver_list_t* hashtab, char *s);
static uint32_t nameserver_find_slot(nameserver_list_t* hashtab, char *name);
static void nameserver_insert(nameserver_list_t* hashtab, char* name, tid_t tid);


/***************************************/
//        NAMESERVER USER TASK
/***************************************/

void nameserver_task(void) {
    nameserver_list_t hashtab[MAX_NAME_SERVER_NAMES];

    nameserver_initialize(hashtab, MAX_NAME_SERVER_NAMES);

    FOREVER {
        //TODO put request handling code here
    }

    //We shouldn't ever get here
    ASSERT(0);
}


/***************************************/
//   NAMESERVER HASH TABLE FUNCTIONS
/***************************************/

/* hash: form hash value for string s */
uint32_t hash(char *s) {
    uint32_t hashval;
    for(hashval = 0; *s != '\0'; ++s) {
        hashval = *s + 31 * hashval;
    }
    return hashval % MAX_NAME_SERVER_NAMES;
}

void nameserver_initialize(nameserver_list_t* hashtab, size_t hashtab_size) {
    size_t i;
    for(i = 0; i < hashtab_size; ++i) {
        hashtab[i].filled = false;
    }
}

/* lookup: look for s in hashtab */
tid_t nameserver_lookup(nameserver_list_t* hashtab, char *s) {
    uint32_t i = nameserver_find_slot(hashtab, s);

    bwprintf(COM2,"LOOKUP %s\r\n",hashtab[i].name);

    if(hashtab[i].filled == true) {
        return hashtab[i].tid;
    }
    
    return -1;
}

uint32_t nameserver_find_slot(nameserver_list_t* hashtab, char *name) {
    uint32_t i = hash(name);

    while(hashtab[i].filled == true && strcmp(name, hashtab[i].name) != 0) {
        i = (i+1) % MAX_NAME_SERVER_NAMES;
    }

    bwprintf(COM2,"INDEX %d\r\n",i);
    return i;
}

void nameserver_insert(nameserver_list_t* hashtab, char * name, tid_t tid) {
    uint32_t i = nameserver_find_slot(hashtab, name);

    if(hashtab[i].filled) {
        //overwrites the old tid for this name
        hashtab[i].tid = tid;
        return;
    }

    strlcpy(hashtab[i].name, name, MAX_TASK_NAME_LENGTH - 1);
    
    hashtab[i].name[MAX_TASK_NAME_LENGTH-1] = '\0';
    
    bwprintf(COM2,"STINSERT %s\r\n", hashtab[i].name);
    hashtab[i].tid = tid;
    hashtab[i].filled = true;
}
