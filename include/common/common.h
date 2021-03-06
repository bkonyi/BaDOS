#ifndef __COMMON_H__
#define __COMMON_H__

#include <bwio.h>
#include <terminal/terminal_debug_log.h>
#include <syscalls.h>
#include <terminal/terminal_debug_log.h>
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

typedef int32_t tid_t;
typedef uint8_t priority_t;

#define MAX_NUMBER_OF_TASKS 200 //TODO Arbitrary number for now
#define SCHEDULER_NUM_QUEUES            32

#define SENSOR_MESSAGE_SIZE (sizeof(int8_t)*10 + 2+ sizeof(uint32_t))
//\e[2J \e[1;1H

#ifdef ASSERTIONS
	#define ASSERT(cond)  if(!(cond)) { Delay(100); send_term_debug_log_msg("ASSERT FAILED : %s:%d\r\n", __FILE__, __LINE__); Delay(100); Terminate(); } while(0)
	#define KASSERT(cond) if(!(cond)) { bwprintf(COM2, "KASSERT FAILED: %s:%d\r\n", __FILE__, __LINE__); bwgetc(COM2); } while(0)
#else
	#define ASSERT(cond)  ((void)(cond));
	#define KASSERT(cond) ((void)(cond));
#endif


void* memcpy(void* dest, void* src, size_t len);
void* memset(void *s, int c, unsigned int n);

int max(int val1, int val2);
int min(int val1, int val2);

#define CARRIAGE_RETURN (char)13
#define BACKSPACE       (char)8

char* strcpy(char* dest, char* src);
char* strlcpy(char* dest, char* src, uint32_t maxlen);
size_t strlen(char* str);
size_t strnlen(char* str,size_t maxlen);
int32_t strcmp(char* a, char* b);

int8_t sensor_to_id(char* sensor_name);
char sensor_id_to_letter(int8_t id);
int8_t sensor_id_to_number(int8_t id);

/**
 * @brief modifies the given string so that words inside of it can be accessed as tokens
 * @details turns all whitespace into \0, and modifies argv so that it contains a list of
 * pointers to the beginning of those tokens.
 * 
 * @param str the string to tokenize
 * @param argv an array of size maxtoks
 * @param maxtoks the maximum number of tokens that can be held in argv
 * 
 * @return on success, returns the number of tokens that have been inserted into argv
 * 				-1 if more tokens where found than could fit into argv
 */
int32_t strtokenize(char* str, char** argv, uint32_t maxtoks);

/**
 * @brief takes a decimal or hexadecimal string and returns it's value as an integer
 * 
 * @param c the string of characters to convert to an integer
 * @return returns the integer that the string converts to. Confusingly, it will return -1
 *  if the string is ill formatted. So that error message is only really useful if 
 *  you expect non negative numbers
 */
int strtoi(char* c);
char char_to_upper(char a);
void str_to_upper(char* a);

//TODO: this common file needs to be split into kcommon, common, and ucommon since rand uses a syscall
uint32_t rand(void);

void sprintf( char* dest, char *format, ... );

void svprintf(char* dest, char* fmt, va_list va);

uint32_t sqrt(uint64_t const n);

/**
 * Structure necessary to represent a queue of task descriptors.
 */
#define WHOIS_ID        ((char)0x20)
#define REGISTERAS_ID   ((char)0x40)
#define NAMESERVER_TID  0x1
 
typedef struct {
    char send_id;
    tid_t tid;
    char* name;
} nameserver_msg_t;

#endif
