#ifndef BOB_HASH_H
#define BOB_HASH_H

#include <stdint.h>
#include <sys/types.h>
#define MAX_PRIME 1229
uint32_t bobhash(uint32_t primeNum, const unsigned char * str, size_t len);
#endif

