#ifndef __COMMON_H__
#define __COMMON_H__

#include <bwio.h>
#include <syscalls.h>

#define FOREVER for(;;)

#define NULL 0

typedef unsigned int size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef enum { false, true } bool;

typedef int16_t tid_t;
typedef uint8_t priority_t;

#define ASSERT(cond)  if(!cond) { bwprintf(COM2, "ASSERT FAILED : %s:%d\r\n", __FILE__, __LINE__); Exit(); } while(0)
#define KASSERT(cond) if(!cond) { bwprintf(COM2, "KASSERT FAILED: %s:%d\r\n", __FILE__, __LINE__); bwgetc(COM2); } while(0)

void* memcpy(void* dest, void* src, size_t len);
int max(int val1, int val2);
int min(int val1, int val2);

char* strcpy(char* dest, char* src);
char* strlcpy(char* dest, char* src, uint32_t maxlen);
size_t strlen(char* str);
int32_t strcmp(char* a, char* b);
void to_upper(char* a);

/**
 * Structure necessary to represent a queue of task descriptors.
 */
#define WHOIS_ID        ((char)0x20)
#define REGISTERAS_ID   ((char)0x40)
#define NAMESERVER_TID  0x0
typedef struct {
    char send_id;
    tid_t tid;
    char* name;
}nameserver_msg_t;

#endif
