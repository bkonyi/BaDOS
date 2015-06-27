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
#include <io/uart_servers.h>

#define SEND_TRANSMIT_BUFFER() { 											     \
	Send(transmit_server_tid, msg_buffer, msg_buffer_index, (char*)NULL, 0); 	 \
	msg_buffer_index = 0;														 \
} while(0)

static int buffer_putw( char* output_buffer, int buffer_size, int n, char fc, char *bf );

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

int buffer_putw( char* output_buffer, int buffer_size, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	int index = 0;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) {
		if(index >= buffer_size) {
			return -1;
		}

		output_buffer[index++] = fc;
	}

	while( ( ch = *bf++ ) ) {
		if(index >= buffer_size) {
			return -1;
		}
		
		output_buffer[index++] = ch;
	}

	return index;
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

	char msg_buffer[TRANSMIT_BUFFER_SIZE];
	int msg_buffer_index = 0;

	int result;
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

	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' ) {
			msg_buffer[msg_buffer_index++] = ch;
		} else {
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
			case 0:
				SEND_TRANSMIT_BUFFER(); 
				return;
			case 'c':
				msg_buffer[msg_buffer_index++] = va_arg( va, char );
				break;
			case 's':
				do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, 0, va_arg(va, char*));
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);

				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);
				break;
			case '%':
				msg_buffer[msg_buffer_index++] = ch;
				break;
			}
		}

		if(msg_buffer_index == TRANSMIT_BUFFER_SIZE) {
			SEND_TRANSMIT_BUFFER();
		}
	}

	SEND_TRANSMIT_BUFFER();
}
void strformat ( char* dest, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	char* msg_buffer = dest;
	char* in_buff;
	


	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' ) {
			*msg_buffer = ch;
			msg_buffer++;
		} else {
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
			case 0:
				*msg_buffer = '\0';
				msg_buffer++; 
				return;
			case 'c':
				*msg_buffer = va_arg( va, char );
				msg_buffer++;
				break;
			case 's':
				in_buff = va_arg(va, char*);
				strcpy(msg_buffer,in_buff);
				msg_buffer+=strlen(in_buff);
				/*do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, 0, va_arg(va, char*));
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);*/

				break;
			case 'u':
				ui2a( va_arg( va, unsigned int ), 10, bf );
				strcpy(msg_buffer,bf);
				msg_buffer+=strlen(bf);
				/* do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1); */
				break;
			case 'd':
				i2a( va_arg( va, int ), bf );
				strcpy(msg_buffer,bf);
				msg_buffer+=strlen(bf);
				/*do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);*/
				break;
			case 'x':
				ui2a( va_arg( va, unsigned int ), 16, bf );
				strcpy(msg_buffer,bf);
				msg_buffer+=strlen(bf);
				/*do {
					result = buffer_putw(&msg_buffer[msg_buffer_index], TRANSMIT_BUFFER_SIZE - msg_buffer_index, w, lz, bf);
					if(result == -1) {
						SEND_TRANSMIT_BUFFER();
					} else {
						msg_buffer_index += result;
					}
				} while(result == -1);*/
				break;
			case '%':
				*msg_buffer = ch;
				msg_buffer++;
				break;
			}
		}
	}
	*msg_buffer = '\0';
}


void printf( int channel, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        format( channel, fmt, va );
        va_end(va);
}
void vprintf(int channel, char* fmt, va_list va){
    format( channel, fmt, va );
}

void sprintf( char* dest, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        strformat( dest, fmt, va );
        va_end(va);
}
void vsprintf(char* dest, char* fmt, va_list va){
    strformat( dest, fmt, va );
}
