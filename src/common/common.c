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

uint32_t rand(void) {
    uint32_t rand_val;
    tid_t tid;

    do {
        tid = WhoIs(RAND_SERVER);
    } while(tid == -2);

    Send(tid, NULL, 0, (void*)(&rand_val), sizeof(uint32_t));

    return rand_val;
}

