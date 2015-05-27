#include <nameserver.h>
#include <common.h>
#include <global.h>
#include "bwio.h"

#define MAX_TASK_NAME_LENGTH  20
#define MAX_NAME_SERVER_NAMES 200

typedef struct { /* table entry: */
    char name[MAX_TASK_NAME_LENGTH]; /* defined name */
    tid_t tid;
    bool filled; /* replacement text */
} nameserver_list_t;


//Forward declarations of nameserver hash table functions

/**
 * @brief gets an index for the input name, s, which is in the range
 * [0,MAX_NAME_SERVER_NAMES)
 * 
 * @param s The null terminated character array to find a has for
 * @return the index for the hash, in the range 
 * [0,MAX_NAME_SERVER_NAMES)
 */
static uint32_t hash(char *s);

static void nameserver_initialize(nameserver_list_t* hashtab, size_t hashtab_size);
/**
 * @brief Used to get the tid of an associated name, in hashtab
 * 
 * @param hashtab the hashtable to search through
 * @param s character array holding a null terminated name, to be
 * found in hashtab
 * 
 * @return 
 * -1 if the hash s isn't found in the hash table
 * -2 if the hash list is full and
 */
static tid_t nameserver_lookup(nameserver_list_t* hashtab, char *s);

/**
 * @brief Given a hash table and a null terminated character array,
 * finds the index in the hash table where the name entry 
 * belongs
 * @details this is done using linear open addressing. The index 
 * of a preexisting entry of name in the hashtab, or the first 
 * available index
 * 
 * @param hashtab The hashtable in concern
 * @param name null terminated char array, the key in the hash table
 * 
 * @return 
 * the index corresponding the name in the hashtab
 * -2 if the hashtable is full
 */
static int32_t nameserver_find_slot(nameserver_list_t* hashtab, char *name);

/**
 * @brief inserts the value, tid, into the hash table, hashtab, 
 * based on the key, name.
 * @details this is done using linear open addressing. The value will
 * be inserted overtop a previous entry for the key, name, 
 * or insterted in the first available slot
 * 
 * @param hashtab The hashtable in concern
 * @param name null terminated char array, the key in the hash table
 * @param tid the tid, value, to be inserted in the hashtab
 * @return 
 * 0 on success
 * -2 if the hashtab is full
 */
static int32_t nameserver_insert(nameserver_list_t* hashtab, char* name, tid_t tid);


/***************************************/
//        NAMESERVER USER TASK
/***************************************/

void nameserver_task(void) {

    int sender_tid;
    volatile nameserver_msg_t msg;
    nameserver_list_t hashtab[MAX_NAME_SERVER_NAMES];
    int result;
    nameserver_initialize(hashtab, MAX_NAME_SERVER_NAMES);

   FOREVER {
        Receive( &sender_tid, (char*)&msg, sizeof(nameserver_msg_t));
        switch(msg.send_id){
            case WHOIS_ID:
                result = nameserver_lookup(hashtab,msg.name);
                Reply( sender_tid, 
                    (char*)&result, 
                    sizeof(tid_t));
                break;
            case REGISTERAS_ID:
                result = nameserver_insert(hashtab,msg.name,msg.tid);
                Reply(sender_tid,(char*)&result,sizeof(int));
                break;
        }
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
    //Set all entries so that they are empty
    for(i = 0; i < hashtab_size; ++i) {
        hashtab[i].filled = false;
    }
}

/* lookup: look for s in hashtab */
tid_t nameserver_lookup(nameserver_list_t* hashtab, char *s) {
    uint32_t i = nameserver_find_slot(hashtab, s);
    if(i==-2) return -2; // Our hash list is full 
    //otherwise check if a value has been entered at this index 
    if(hashtab[i].filled == true) {
        return hashtab[i].tid;
    }
    
    return -1;
}

int32_t nameserver_find_slot(nameserver_list_t* hashtab, char *name) {
    uint32_t i = hash(name);
    uint32_t orig_i = i;
    //starting at the index given by hash, look for an entry that 
    //already exists with this key, or the first open slot
    while(hashtab[i].filled == true && strcmp(name, hashtab[i].name) != 0) {
        i = (i+1) % MAX_NAME_SERVER_NAMES;
        if(i == orig_i)return -2; // looks like the hash_tab is full
    }
    return i;
}

int32_t nameserver_insert(nameserver_list_t* hashtab, char * name, tid_t tid) {
    uint32_t i = nameserver_find_slot(hashtab, name);
    if(i==-2) return -2;
    if(hashtab[i].filled) {
        //overwrites the old tid for this name
        hashtab[i].tid = tid;
        return 0;
    }
    //Copy the string to the hashtab
    strlcpy(hashtab[i].name, name, MAX_TASK_NAME_LENGTH - 1);
    
    //enforce that our names are null terminated
    hashtab[i].name[MAX_TASK_NAME_LENGTH-1] = '\0';
    //add the rest of the info
    hashtab[i].tid = tid;
    hashtab[i].filled = true;
    return 0;
}
