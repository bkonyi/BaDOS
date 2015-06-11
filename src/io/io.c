/*
 * io.c - interrupt driven I/O routines
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include <ts7200.h>
#include <common.h>
#include <io.h>
#include <io_common.h>
#include <servers.h>
#include <syscalls.h>

int putc( int channel, char c ) {
	int transmit_server_tid = -1;

	switch(channel) {
		case COM1:
			transmit_server_tid = UART1_TRANSMIT_SERVER_ID;
			break;
		case COM2:
			transmit_server_tid = UART2_TRANSMIT_SERVER_ID;
			break;
		default:
			ASSERT(0); //Do we want to do this?
	}

	Send(transmit_server_tid, &c, sizeof(char), (char*)NULL, 0);

	return 0;
}

int putx( int channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	putc( channel, chh );
	return putc( channel, chl );
}

int putr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) putx( channel, ch[byte] );
	return putc( channel, '\n' );
}

int putstr( int channel, char *str ) {
	while( *str ) {
		if( putc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void putw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) putc( channel, fc );
	while( ( ch = *bf++ ) ) putc( channel, ch );
}

int getc( int channel ) {
	int transmit_server_tid = -1;
 	char byte;
	switch(channel) {
		case COM1:
			transmit_server_tid = UART1_RECEIVE_SERVER_ID;
			break;
		case COM2:
			transmit_server_tid = UART2_RECEIVE_SERVER_ID;
			break;
		default:
			ASSERT(0); //Do we want to do this?
	}

	Send(transmit_server_tid, (char*)NULL, 0, &byte, sizeof(char));

	return (int)byte;
}

void format ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			putc( channel, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				putc( channel, va_arg( va, char ) );
				break;
			case 's':
				putw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				putw( channel, w, lz, bf );
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				putw( channel, w, lz, bf );
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				putw( channel, w, lz, bf );
				break;
			case '%':
				putc( channel, ch );
				break;
			}
		}
	}
}

void printf( int channel, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        format( channel, fmt, va );
        va_end(va);
}
