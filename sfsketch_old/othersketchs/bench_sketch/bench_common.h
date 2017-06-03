#ifndef _BENCH_COMMON_H_
#define _BENCH_COMMON_H_
//#include "../fcsketch/sketch_config.h"
#include "bench_config.h"

#define UPDATE_FILE "operations.dat"
#define QUERY_FILE "queries.dat"



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
