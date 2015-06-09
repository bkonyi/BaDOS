#include <io_common.h>

/*
 * The UARTs are initialized by RedBoot to the following state
 *  115,200 bps
 *  8 bits
 *  no parity
 *  fifos enabled
 */
int setfifo( int channel, int state ) {
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

int setspeed( int channel, int speed ) {
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
        *low = 0xBF;
        return 0;
    default:
        return -1;
    }
}

int a2d( char ch ) {
    if( ch >= '0' && ch <= '9' ) return ch - '0';
    if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
    if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
    return -1;
}

char a2i( char ch, char **src, int base, int *nump ) {
    int num, digit;
    char *p;

    p = *src; num = 0;
    while( ( digit = a2d( ch ) ) >= 0 ) {
        if ( digit > base ) break;
        num = num*base + digit;
        ch = *p++;
    }
    *src = p; *nump = num;
    return ch;
}

void ui2a( unsigned int num, unsigned int base, char *bf ) {
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

void i2a( int num, char *bf ) {
    if( num < 0 ) {
        num = -num;
        *bf++ = '-';
    }
    ui2a( num, 10, bf );
}
