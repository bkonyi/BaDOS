#ifndef __IO_COMMON_H__
#define __IO_COMMON_H__

#include <ts7200.h>

typedef char *va_list;

#define __va_argsiz(t)  \
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)  ((void)0)

#define va_arg(ap, t)   \
         (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

#define COM1    0
#define COM2    1

#define ON  1
#define OFF 0

char c2x( char ch );

int a2d( char ch );

char a2i( char ch, char **src, int base, int *nump );

void ui2a( unsigned int num, unsigned int base, char *bf );

void i2a( int num, char *bf );

#endif //__IO_COMMON_H__
