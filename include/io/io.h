#ifndef __IO_H__
#define __IO_H__
/*
 * io.h
 * These functions use interrupt based IO.
 */

#include <io_common.h>

#define COM1	0
#define COM2	1

#define ON	1
#define	OFF	0

int setfifo( int channel, int state );

int setspeed( int channel, int speed );

int putc( int channel, char c );

int getc( int channel );

int putx( int channel, char c );

int putstr( int channel, char *str );

int putr( int channel, unsigned int reg );

void putw( int channel, int n, char fc, char *bf );

void printf( int channel, char *format, ... );

void vprintf(int channel, char* fmt, va_list va);

void sprintf( char* dest, char *fmt, ... ) ;
void vsprintf(char* dest, char* fmt, va_list va);

#endif //__IO_H__
