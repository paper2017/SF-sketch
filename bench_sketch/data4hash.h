#ifndef _DATA_HASH_H_
#define _DATA_HASH_H_
#include <stdint.h>
typedef int32_t int32;
/*
 * format of each request, it has a key and a type and we don't care
 * the value
 */
typedef struct __attribute__((__packed__)) {
    char key[8];
    int32 value;
} requestA;

typedef struct __attribute__((__packed__)) {
    char key[16];
    int32 value;
} requestB;

typedef struct __attribute__((__packed__)) {
    char key[128];
    int32 value;
} requestC;

int32 stol(char *value) {
    return (int32)(value[0]<<24) + (int32)(value[1]<<16) + (int32)(value[2]<<8) + (int32)value[3];
}

void toRechar(char *value, int length) {
    //33 126

    unsigned int a;
    for(int i=0; i<length; i++){
        a = (unsigned int)value[i];
        if(a < 33 || a > 126) {
            value[i] = a % 94 + 33;
        }
    }
}

void lltos(char *str, uint64_t value) {
    str[0] = value >> 56;
    str[1] = value >> 48;
    str[2] = value >> 40;
    str[3] = value >> 32;
    str[4] = value >> 24;
    str[5] = value >> 16;
    str[6] = value >> 8;
    str[7] = value;
}

#endif
