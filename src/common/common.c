#include <common.h>
#include <syscalls.h>
#include <servers.h>
#include <bwio.h>
#include <io.h>
void* memcpy(void* dest, void* src, size_t len) {
        char* dst_c = (char*)dest;
        char* src_c = (char*)src;

        while (len--) {
            *dst_c++ = *src_c++;
        }

        return dest;
}

void *memset(void *s, int c, unsigned int n) {
  unsigned char *p = s;
  while(n --> 0) { *p++ = (unsigned char)c; }
  return s;
}

char* strcpy(char* dest, char* src) {
    memcpy(dest, src, strlen(src)+1);
    return dest;
}

char* strlcpy(char* dest, char* src,uint32_t maxlen) {
    if(maxlen == 0) {
        return strcpy(dest,src);
    }

    size_t len = strlen(src);

    if(len >= maxlen) {
        memcpy(dest, src, maxlen - 1);
        dest[maxlen - 1] = '\0';
    } else {
        memcpy(dest, src, len);
        dest[len] = '\0';
    }

    return dest;
}

size_t strlen(char* str) {
    char* s = str;
    size_t count =0;
    while(*s != '\0') {
        count++;
        s++;
    }
    return count;
}
size_t strnlen(char* str,size_t maxlen) {
    char* s = str;
    size_t count =0;
    while(*s != '\0' && count < maxlen) {
        count++;
        s++;
    }
    return count;
}

char char_to_upper(char a) {
    if('a'<= a && a <= 'z'){
        a = a - ('a'-'A');
    }

    return a;
}

void str_to_upper(char* a) {

    char* A = a;
    while(*A != '\0' ){
        *A = char_to_upper(*A);
        A++;
    }
}

int32_t strcmp(char* a, char* b) {
    char* A = a;
    char* B = b;

    while(*A != '\0' && B != '\0') {
        if(*A < *B) {
            return -1;
        } else if(*A > *B) {
            return 1;
        }
        //Otherwise they must be equal
        A++;
        B++;
    }

    if(*A != '\0') {
        return 1;   //B is shorter
    } else if(*B != '\0') {
        return -1;  //A is shorter
    } else {
        return 0;   // they match
    }
}

int8_t sensor_to_id(char* sensor) {
    char sensor_letter = char_to_upper((sensor)[0]);
    int8_t sensor_number = strtoi(&(sensor[1])) - 1;
    sensor_number += 16 * (sensor_letter - 'A');

    return sensor_number;
}

char sensor_id_to_letter(int8_t id) {
    return (id / 16) + 'A';
}

int8_t sensor_id_to_number(int8_t id) {
    return (id % 16) + 1;
}

int max(int val1, int val2) {
    if(val1 > val2) {
        return val1;
    }

    return val2;
}

int min(int val1, int val2) {
    if(val1 > val2) {
        return val2;
    }

    return val1;
}
static bool is_whitespace(char c) {
    return(c == ' ' || c == (char)CARRIAGE_RETURN);
}
int32_t strtokenize(char* str, char** argv, uint32_t maxtoks){
    //Tokenizing string taken from Dan's(20427084) a0
    char* iter=str;
    uint32_t argc =0; 
    char lastchar =' '; //we want the first char to count
    while(*iter!='\0'){
        if(is_whitespace(*iter)){
            *iter='\0';
            lastchar =' ';
        }else{
            if(lastchar==' '){
                if(argc >= maxtoks)return -1; // we have found too many tokens to fit in argv
                argv[argc]=iter;
                argc++; 
            }
            lastchar=*iter;
        }
        iter++;
    }   
    return argc;

}
static int char2int( char ch, uint32_t base ) {
    if( ch >= '0' && ch <= '9' && base >= 10) return ch - '0';
    if( ch >= 'a' && ch <= 'f' && base == 16) return ch - 'a' + 10;
    if( ch >= 'A' && ch <= 'F' && base == 16) return ch - 'A' + 10;
    return -1;
}

int strtoi(char* c) {
    int num;
    int charnum;
    int base = 10;
    bool is_neg = false;
    num =0;
    if(c[0]!= '\0' && c[1]!= '\0' && c[0] =='0' && c[1] =='x'){
        if(c[2]=='\0') return -1; // Improper hex format
        //This is hex, skip the first 2 format chars
        c+=2; //
        base = 16;
    }else if(c[0] == '-') {
        c++;
        is_neg = true;
    }
    for(;*c!='\0';c++){
        num*=base;
        charnum = char2int(*c, base);
        if(charnum < 0) return -1;
        num+=charnum;
    }
    if(is_neg == true) {
        num *= -1;
    }
    return num;

}

uint32_t rand(void) {
    uint32_t rand_val;
    tid_t tid;

    do {
        tid = WhoIs(RAND_SERVER);
    } while(tid == -2);

    Send(tid, NULL, 0, (void*)(&rand_val), sizeof(uint32_t));

    return rand_val;
}

//Note: Once again, don't as me how.
//http://stackoverflow.com/questions/21657491/an-efficient-algorithm-to-calculate-the-integer-square-root-isqrt-of-arbitrari
uint32_t sqrt(uint64_t const n) {
    uint64_t xk = n;
    if (n == 0) return 0;
    if (n == 18446744073709551615ULL) return 4294967295U;
    do {
        uint64_t const xk1 = (xk + n / xk) / 2;
        if (xk1 >= xk) {
            return xk;
        } else {
            xk = xk1;
        }
    } while (1);
}
