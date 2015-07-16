#include <priority_heap.h>

//NOTE: this priority heap implementation is based off of this implementation:
// https://github.com/kartikkukreja/blog-codes/blob/master/src/Indexed%20Min%20Priority%20Queue.cpp

#define LEFT(x) 2 * (x)
#define RIGHT(x) LEFT(x) + 1
#define PARENT(x) (x) / 2

static void swap(priority_heap_t* min_heap, int i, int j);
static void bubbleUp(priority_heap_t* min_heap, int k);
static void bubbleDown(priority_heap_t* min_heap, int k);

void swap(priority_heap_t* min_heap, int i, int j) {
    int* heap = min_heap->heap;
    int* index = min_heap->index;

    int t = heap[i]; heap[i] = heap[j]; heap[j] = t;
    index[heap[i]] = i; index[heap[j]] = j;
}

void bubbleUp(priority_heap_t* min_heap, int k) {
    int* heap = min_heap->heap;
    priority_heap_node_t* nodes = min_heap->nodes;

    while(k > 1 && nodes[heap[PARENT(k)]].value > nodes[heap[k]].value) {
        swap(min_heap, k, PARENT(k));
        k = PARENT(k);
    }
}

void bubbleDown(priority_heap_t* min_heap, int k) {
    int* heap = min_heap->heap;
    priority_heap_node_t* nodes = min_heap->nodes;

    int j;
    while(LEFT(k) <= min_heap->size) {
        j = LEFT(k);
        if(j < min_heap->size && nodes[heap[j]].value > nodes[heap[j + 1]].value)
            j++;
        if(nodes[heap[k]].value <= nodes[heap[j]].value)
            break;
        swap(min_heap, k, j);
        k = j;
    }
}

void priority_heap_init(priority_heap_t* min_heap) {
    ASSERT(min_heap != NULL);

    int i;
    for(i = 0; i < PRIORITY_QUEUE_SIZE; ++i) {
        min_heap->index[i] = -1;
        min_heap->heap[i] = -1;
    }

    min_heap->size = 0;
}

// check if the PQ is empty
bool isEmpty(priority_heap_t* min_heap) {
    return (min_heap->size == 0);
}

// check if i is an index on the PQ
bool contains(priority_heap_t* min_heap, int i) {
    return (min_heap->index[i] != -1);
}

// return the number of elements in the PQ
int size(priority_heap_t* min_heap) {
    return min_heap->size;
}

// associate key with index i; 0 < i < NMAX
void insert(priority_heap_t* min_heap, int i, priority_heap_node_t key) {
    min_heap->size++;
    min_heap->index[i] = min_heap->size;
    min_heap->heap[min_heap->size] = i;
    min_heap->nodes[i] = key;
    bubbleUp(min_heap, min_heap->size);
}

// returns the index associated with the minimal key
int minIndex(priority_heap_t* min_heap) {
    return min_heap->heap[1];
}

// returns the minimal key
priority_heap_node_t minKey(priority_heap_t* min_heap) {
return min_heap->nodes[min_heap->heap[1]];
}

// delete the minimal key and return its associated index
// Warning: Don't try to read from this index after calling this function
int deleteMin(priority_heap_t* min_heap) {
    int min = min_heap->heap[1];
    swap(min_heap, 1, min_heap->size--);
    bubbleDown(min_heap, 1);
    min_heap->index[min] = -1;
    min_heap->heap[min_heap->size +1] = -1;
    return min;
}
// returns the key associated with index i
priority_heap_node_t keyOf(priority_heap_t* min_heap, int i) {
    return min_heap->nodes[i];
}

// change the key associated with index i to the specified value
void changeKey(priority_heap_t* min_heap, int i, priority_heap_node_t key) {
    min_heap->nodes[i] = key;
    bubbleUp(min_heap, min_heap->index[i]);
    bubbleDown(min_heap, min_heap->index[i]);
}

// decrease the key associated with index i to the specified value
void decreaseKey(priority_heap_t* min_heap, int i, priority_heap_node_t key) {
    min_heap->nodes[i] = key;
    bubbleUp(min_heap, min_heap->index[i]);
}

// increase the key associated with index i to the specified value
void increaseKey(priority_heap_t* min_heap, int i, priority_heap_node_t key) {
    min_heap->nodes[i] = key;
    bubbleDown(min_heap, min_heap->index[i]);
}

// delete the key associated with index i
void deleteKey(priority_heap_t* min_heap, int i) {
    int ind = min_heap->index[i];
    swap(min_heap, ind, min_heap->size--);
    bubbleUp(min_heap, ind);
    bubbleDown(min_heap, ind);
    min_heap->index[i] = -1;
}
