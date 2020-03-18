#ifndef _BENCH_COMMON_H_
#define _BENCH_COMMON_H_
//#include "../sfsketch/sketch_config.h"
#include "bench_config.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
/*
#define bool char
#define false 0
#define true 1
*/


//#define CHECK_ABNORMAL
//#ifdef NDEBUG 1
//#define DEBUG 1
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
/*
uint32_t hash(uint32_t row, const unsigned char * str, size_t len);
inline uint32_t hash(uint32_t row, const unsigned char * str, size_t len) {
    return bobhash(row, str, len);
}
inline uint32_t hash_l(uint32_t row, const unsigned char * str, size_t len) {
    return bobhash(row + MAX_ROW, str, len);
}*/
/* type of each request */
enum request_types{
    request_init=0,
    request_inc,
    request_dec,
    request_get
};

/*
 * format of each request, it has a key and a type and we don't care
 * the value
 */
typedef struct __attribute__((__packed__)) {
    char hashed_key[NKEY];
    char type;
    uint8_t delta;
} request;

/* bench result */
typedef struct __attribute__((__packed__)) {
    double total_tput;
    double total_time;
    size_t total_hits;
    size_t total_miss;
    size_t total_gets;
    size_t total_puts;
    size_t num_threads;
} result_t;
#endif
