#include <common.h>

void* memcpy(void* dest, void* src, size_t len) {
        char* dst_c = (char*)dest;
        char* src_c = (char*)src;

        while (len--) {
            *dst_c++ = *src_c++;
        }

        return dest;
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
