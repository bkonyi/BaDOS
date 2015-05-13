#ifndef __COMMON_H__
#define __COMMON_H__

#define NULL 0

#define TRUE  1
#define FALSE 0

typedef unsigned int size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

/**
 * @brief Initializes memory management system
 * @details Initializes memory management system.
 */
void init_memory(void);

/**
 * @brief Allocate memory on the heap.
 * @details Allocates a specific amount of memory from the heap and returns the address.
 * 
 * @param bytes Number of bytes to allocate
 * @return NULL if the heap is out of memory, a pointer to the valid memory on the heap otherwise.
 */
void* kmalloc(size_t bytes);

#endif
