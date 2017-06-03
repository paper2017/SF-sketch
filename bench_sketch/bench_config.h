#ifndef __BENCH_CONFIG__
#define __BENCH_CONFIG__

/*
 * size of the key in bytes
 */
#define NKEY 20
#define NVAL 32

#define ONLY_ONE_OPERATE 1

//#define UNIFORM_OPERATE 1

#ifndef FC_V1
#define BITS_DELTA  0
#define ZERO_DELTA  0   //for inc
//#define ZERO_DELTA  4 //for dec//tmp
//#define BITS_DELTA 4//tmp
//#define ZERO_DELTA  16
#else
#define BITS_DELTA 4
#define ZERO_DELTA 0
//use thoes two config bellow will create inc 1 only
//#define BITS_DELTA 0
//#define ZERO_DELTA 0
#endif
#define GAP_DELTA (1 << BITS_DELTA)

#endif
