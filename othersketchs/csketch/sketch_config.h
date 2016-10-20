#ifndef SKETCH_CONFIG_H
#define SKETCH_CONFIG_H
#include <stdio.h>
#include "sketch.h"
#include "BOBHash.h"

#define DEBUG 1
#include <assert.h>

#ifdef DEBUG
#define dbg_printf(...) {\
fprintf(stdout, __VA_ARGS__);\
}
#define dbg_vprintf(...) {\
fprintf(stdout, "\nFile:%s, Line:%d, Function:%s\n", __FILE__, __LINE__ , __FUNCTION__);\
fprintf(stdout, __VA_ARGS__);\
}
#else
#define dbg_printf(...)
#define dbg_vprintf(...)
#endif // DEBUG

#define MAX_ROW (MAX_PRIME / 2)

#define MAX_D MAX_ROW
#define MAX_WL 1024*1024*8
#define MAX_Z 128
//MAX_BITS_C can be set as number of bits for a machine word
#define MAX_BITS_C (sizeof(int64_t)*8)
#define SZ_ALIGN(addr) ((addr) & (~0 << ALIGN_SHIFT))

#define SUPPORT_DELETE
#define NEEDHASH2

typedef enum sketch_name {
    SLIM_SKETCH,
    FAT_SKETCH
} sketch_name;

typedef struct sketch_l_t {
    size_t D;
    size_t W;

    //counter:
    size_t bits_c;       // Memory bits for a single counter
    size_t max_c;        // The maximum value for Upper limit for a counter
    size_t length;       // Memory size(unit: sizeof(size_t)) for all the counters

    //sketch:
    size_t * counts[];
} sketch_l;

uint32_t hash(uint32_t row, const unsigned char * str, size_t len);

#ifdef NEEDHASH2
uint32_t hash2(uint32_t row, const unsigned char * str, size_t len);
#endif



#endif // SKETCH_CONFIG_H
