#ifndef SKETCH_CONFIG_H
#define SKETCH_CONFIG_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>
#include "BOBHash.h"
//#include "../sfsketch/sketch_config.h"
/*
#define bool char
#define false 0
#define true 1
*/

//define version:
#define SF_V1 1
//#define SF_V2 1
////#define SF_V3 1
////#define SF_V4 1
#define SF_FINAL 1

#ifdef SF_V1
#define SF_SET_MIN_COUNTER
#define SF_SET_LEFT_HASHS
#endif

#ifdef SF_V2
#define SF_SET_SUPPORT_DELETE
#define SF_SET_EXP_Z
//this setting can improve accuracy for sketch.
#define SF_SET_MIN_COUNTER
#endif

//this version is deprecated.
/*
#ifdef SF_V3
#define SF_SET_SUPPORT_DELETE
#define SF_SET_EXP_Z
#endif
*/

//feature added:
#ifdef SF_FINAL
#define SF_AGING 1
#define SF_SET_SUPPORT_DELETE
#define SF_SET_EXP_Z    //in the paper, we use z, in this program, we use k temporarily.
//#define SF_SET_MIN_COUNTER
//#define SF_SET_MAX_COUNTERS_ARRAY
//#define FAST_VX
#endif

#ifdef SF_SET_EXP_Z
#ifndef SF_SET_DIVISIBLE_Z
#define SF_SET_DIVISIBLE_Z
#endif
//in fast, this can work as long as SF_SET_EXP_Z is defined
#define FAST_VX
#endif

#ifdef SF_SET_SUPPORT_DELETE
#ifndef SF_SET_DIVISIBLE_Z
#define SF_SET_DIVISIBLE_Z
#endif
#endif

//for SUPPORT DELETE, we always use MAX COUNTERS for dec_r even SF_SET_MAX_COUNTERS_ARRAY is not defined,
//and use MAX COUNTER for inc_r only when SF_SET_MAX_COUNTERS_ARRAY is defined.
#ifdef SF_SET_MAX_COUNTERS_ARRAY
#ifdef SF_SET_MIN_COUNTER
#undef SF_SET_MIN_COUNTER
#endif
#endif


//for SF_AGING, we always use MAX COUNTERS for compose_slime, and not use MAX COUNTERS for inc_r,
//and also not use MIN COUNTER for inc_r or dec_r.
#ifdef SF_AGING
#ifdef SF_SET_MIN_COUNTER
#undef SF_SET_MIN_COUNTER
#endif
#ifdef SF_SET_MAX_COUNTERS_ARRAY
#undef SF_SET_MAX_COUNTERS_ARRAY
#endif
#endif

#ifdef SF_V1
#define MAX_ROW (MAX_PRIME / 2)
#else
#define MAX_ROW MAX_PRIME
#endif

#if (SF_V1 + SF_V2 + SF_FINAL != 1)
#error "you must specify one and only one version"
#endif


#define COMPACT_COUNTER_L 1
#define COMPACT_COUNTER_R 1

uint32_t hash(uint32_t row, const unsigned char * str, size_t len);
//#ifdef SF_V1
#ifdef SF_SET_LEFT_HASHS
uint32_t hash_l(uint32_t row, const unsigned char * str, size_t len);
#endif

#endif // SKETCH_CONFIG_H
