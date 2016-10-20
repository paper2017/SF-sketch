#ifndef SKETCH_CONFIG_H
#define SKETCH_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
/*
#define bool char
#define false 0
#define true 1
*/

#include "BOBHash.h"

//#ifdef NDEBUG 1
#define DEBUG 1
#include <assert.h>

/* For DEBUG easily*/
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


//#define FC_V1 1
//#define FC_V2 1
//#define FC_V3 1
#define FC_V4 1

#ifdef FC_V1
#define MAX_ROW (MAX_PRIME / 2)
#else
#define MAX_ROW MAX_PRIME
#endif

#if (FC_V1 + FC_V2 + FC_V3 + FC_V4 != 1)
#error "you must specify one and only one version"
#endif


#define COMPACT_COUNTER_L 1
#define COMPACT_COUNTER_R 1

uint32_t hash(uint32_t row, const unsigned char * str, size_t len);
#ifdef FC_V1
uint32_t hash_l(uint32_t row, const unsigned char * str, size_t len);
#endif

#endif // SKETCH_CONFIG_H
