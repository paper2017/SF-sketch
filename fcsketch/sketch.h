#ifndef SKETCH_H
#define SKETCH_H

#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)
//#define likely(x)   x
//#define unlikely(x) x

#include "sketch_config.h"

#define MAX_D MAX_ROW
#define MAX_WL 1024*1024*8
// K <= 2^7 = 128
#define MAX_K 128
#define MAX_exp_K 7
//注意，这里MAX_BITS_C应该是机器的位数，16/32/64...
#define MAX_BITS_C (sizeof(size_t)*8)
#define SZ_ALIGN(addr) ((addr) & (~0 << ALIGN_SHIFT))

typedef enum sketch_name {
    LEFT_SKETCH,
    RIGHT_SKETCH
}sketch_name;

typedef struct sketch_l_t {
    size_t D;
    size_t W;

    //counter:
	size_t bits_c;       // Memory bits for a single counter
	size_t max_c;        // Upper limit for a counter
	size_t length;       // Memory size(unit: sizeof(size_t)) for all the counters

	//sketch:
	size_t * counts[];
}sketch_l;

//#if (FC_V1 + FC_V2 == 1)
typedef sketch_l sketch_r;
//#endif
//#ifdef FC_V3
//TODO
//#endif

struct auxi_t {
    size_t w;
    size_t c;
};

typedef struct fcsketch_t {
    size_t D;
    size_t WL;
    size_t WR;
    size_t K;
#ifndef FC_V1
    unsigned int exp_K;  //#exponent of K based on 2 
#endif
    //counter:
	size_t bits_c;       // Memory bits for a single counter
	size_t max_c;        // Upper limit for a counter

	//sketch:
	sketch_l * left;
	sketch_r * right;

	//auxilary data
    struct auxi_t auxis[MAX_D];
#ifndef FC_V3
    size_t aux_minr;
#endif
}fcsketch;
#ifdef FC_V1
bool init(size_t D, size_t WL, float f_K, size_t bits_c);
#else
bool init(size_t D, size_t WL, size_t exp_K, size_t bits_c);
#endif
void inc(const unsigned char * str, size_t len, size_t delta);     //increase by one
#if (FC_V2 + FC_V3 + FC_V4 == 1)
void dec(const unsigned char * str, size_t len, size_t delta);     //decrease by one
#endif

size_t query_slim(const unsigned char * str, size_t len);
size_t query_counter(size_t d, size_t w, sketch_name name);


size_t query(const unsigned char * str, size_t len);

#if (FC_AGING == 1)
void compose_slim(void);
void set_query_slim(void);
void set_query_fat(void);
size_t query_fat(const unsigned char * str, size_t len);
#endif

#endif // SKETCH_H
