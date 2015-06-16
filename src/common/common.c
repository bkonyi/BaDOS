#include <common.h>
#include <syscalls.h>
#include <servers.h>
#include <bwio.h>

void* memcpy(void* dest, void* src, size_t len) {
        char* dst_c = (char*)dest;
        char* src_c = (char*)src;

        while (len--) {
            *dst_c++ = *src_c++;
        }

        return dest;
}
char* strcpy(char* dest, char* src) {
    memcpy(dest, src, strlen(src));
    return dest;
}

char* strlcpy(char* dest, char* src,uint32_t maxlen) {
    if(maxlen == 0) {
        return strcpy(dest,src);
    }
    //TODO: this always copues the maxlen even in \0 is hit.
    memcpy(dest,src,maxlen);
    return dest;

}

size_t strlen(char* str) {
    char* s = str;
    size_t count =0;
    while(*s != '\0') {
        count++;
    }
    return count;
}

char char_to_upper(char a) {
    if('a'<= a && a <= 'z'){
        a = a - ('a'-'A');
    }

    return a;
}

void to_upper(char* a) {
    //TODO: untested
    #if 0
    char* A = a;
    while(*A != '\0' ){
        if('a'<= *A && *A <= 'z'){
            *A = *A +('a'-'A');
        }
        A++;
    }
    #endif
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
    num =0;
    if(c[0]!= '\0' && c[1]!= '\0' && c[0] =='0' && c[1] =='x'){
        if(c[2]=='\0') return -1; // Improper hex format
        //This is hex, skip the first 2 format chars
        c+=2; //
        base = 16;
    }
    for(;*c!='\0';c++){
        num*=base;
        charnum = char2int(*c, base);
        if(charnum < 0) return -1;
        num+=charnum;
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

