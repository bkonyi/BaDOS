#include <common.h>

#define HEAP_SIZE 1024 * 1024 * 16 //16MB

static char HEAP[HEAP_SIZE];
static size_t heap_location;

void init_memory(void) {
    heap_location = 0;
}

void* kmalloc(size_t bytes) {
    if(heap_location + bytes >= HEAP_SIZE) {
        return NULL; //Out of memory
    }

    void* allocated = &HEAP[heap_location];
    heap_location += bytes;

    return allocated;
}
