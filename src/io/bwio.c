/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include <ts7200.h>
#include <bwio.h>

/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */
int bwsetfifo( int channel, int state ) {
	volatile int *line, buf;
	switch( channel ) {
	case COM1:
		line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
	        break;
	case COM2:
	        line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
	        break;
	default:
	        return -1;
	        break;
	}
	buf = *line;
	buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
	*line = buf;
	return 0;
}

int bwsetspeed( int channel, int speed ) {
	volatile int *high, *low;
	switch( channel ) {
	case COM1:
		high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
	        break;
	case COM2:
		high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
	        break;
	default:
	        return -1;
	        break;
	}
	switch( speed ) {
	case 115200:
		*high = 0x0;
		*low = 0x3;
		return 0;
	case 2400:
		*high = 0x0;
		*low = 0x90;
		return 0;
	default:
		return -1;
	}
}

int bwputc( int channel, char c ) {
	volatile int *flags, *data;
	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while( ( *flags & TXFF_MASK ) ) ;
	*data = c;
	return 0;
}

char c2x( char ch ) {
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

int bwputx( int channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	bwputc( channel, chh );
	return bwputc( channel, chl );
}

int bwputr( int channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) bwputx( channel, ch[byte] );
	return bwputc( channel, '\n' );
}

int bwputstr( int channel, char *str ) {
	while( *str ) {
		if( bwputc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void bwputw( int channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) bwputc( channel, fc );
	while( ( ch = *bf++ ) ) bwputc( channel, ch );
}

int bwgetc( int channel ) {
	volatile int *flags, *data;
	unsigned char c;

	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while ( !( *flags & RXFF_MASK ) ) ;
	c = *data;
	return c;
}

int bwa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char bwa2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = bwa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void bwui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void bwi2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	bwui2a( num, 10, bf );
}

void bwformat ( int channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			bwputc( channel, ch );
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
				ch = bwa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				bwputc( channel, va_arg( va, char ) );
				break;
			case 's':
				bwputw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				bwui2a( va_arg( va, unsigned int ), 10, bf );
				bwputw( channel, w, lz, bf );
				break;
			case 'd':
				bwi2a( va_arg( va, int ), bf );
				bwputw( channel, w, lz, bf );
				break;
			case 'x':
				bwui2a( va_arg( va, unsigned int ), 16, bf );
				bwputw( channel, w, lz, bf );
				break;
			case '%':
				bwputc( channel, ch );
				break;
			}
		}
	}
}

void bwprintf( int channel, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        bwformat( channel, fmt, va );
        va_end(va);
}

void bwdumpregs()
{
	unsigned int r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14;

	__asm__(
		"MOV %0,r0\n\t"
		"MOV %1,r1\n\t"
		"MOV %2,r2\n\t"
		"MOV %3,r3\n\t"
		"MOV %4,r4\n\t"
		"MOV %5,r5\n\t"
		"MOV %6,r6\n\t"
		"MOV %7,r7\n\t"
		"MOV %8,r8\n\t"
		"MOV %9,r9\n\t"
		"MOV %10,r10\n\t"
		:"=r"(r0),"=r"(r1),"=r"(r2),"=r"(r3),"=r"(r4),"=r"(r5),"=r"(r6),"=r"(r7),"=r"(r8),"=r"(r9),"=r"(r10));
	__asm__(
		"MOV %0,r11\n\t"
		"MOV %1,r12\n\t"
		"MOV %2,r13\n\t"
		"MOV %3,r14\n\t"
		
		:"=r"(r11),"=r"(r12),"=r"(r13),"=r"(r14));
	bwprintf(COM2,"r0:0x%x\r\n r1:0x%x\r\n r2:0x%x\r\nr3:0x%x\r\n r4:0x%x\r\n r5:0x%x\r\n r6:0x%x\r\n r7:0x%x\r\n r8:0x%x\r\nr9:0x%x\r\n r10:0x%x\r\n r11:0x%x\r\n r12:0x%x\r\n r13:0x%x\r\n r14:0x%x\r\n",r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14);
}
