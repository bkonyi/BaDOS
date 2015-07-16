#ifndef __PRIORITY_HEAP_H__
#define __PRIORITY_HEAP_H__

#include <common.h>
#include <track/track_node.h>
#include <track/track_data.h>

//NOTE: this priority heap implementation is based off of this implementation:
// https://github.com/kartikkukreja/blog-codes/blob/master/src/Indexed%20Min%20Priority%20Queue.cpp

#define PRIORITY_QUEUE_SIZE TRACK_MAX

typedef struct {
    track_node* track_node;
    uint16_t value;
} priority_heap_node_t;

typedef struct {
    int heap[PRIORITY_QUEUE_SIZE];
    int index[PRIORITY_QUEUE_SIZE];
    priority_heap_node_t nodes[PRIORITY_QUEUE_SIZE];
    int16_t size;
} priority_heap_t;

//initialize the structure
void priority_heap_init(priority_heap_t* min_heap);

// check if the PQ is empty
bool isEmpty(priority_heap_t* min_heap);

// check if i is an index on the PQ
bool contains(priority_heap_t* min_heap, int i);

// return the number of elements in the PQ
int size(priority_heap_t* min_heap);

// associate key with index i; 0 < i < NMAX
void insert(priority_heap_t* min_heap, int i, priority_heap_node_t key);

// returns the index associated with the minimal key
int minIndex(priority_heap_t* min_heap);

// returns the minimal key
priority_heap_node_t minKey(priority_heap_t* min_heap);

// delete the minimal key and return its associated index
// Warning: Don't try to read from this index after calling this function
int deleteMin(priority_heap_t* min_heap);

// returns the key associated with index i
priority_heap_node_t keyOf(priority_heap_t* min_heap, int i);

// change the key associated with index i to the specified value
void changeKey(priority_heap_t* min_heap, int i, priority_heap_node_t key);

// decrease the key associated with index i to the specified value
void decreaseKey(priority_heap_t* min_heap, int i, priority_heap_node_t key);

// increase the key associated with index i to the specified value
void increaseKey(priority_heap_t* min_heap, int i, priority_heap_node_t key);

// delete the key associated with index i
void deleteKey(priority_heap_t* min_heap, int i);

#endif //__PRIORITY_HEAP_H__
