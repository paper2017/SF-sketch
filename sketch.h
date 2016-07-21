#ifndef SKETCH_H
#define SKETCH_H
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>

//#define SF1 1
//#define SF2 1
//#define SF3 1
//#define SF4 1
//#define SF5 1
//#define SF6 1

#if (SF1 + SF2 == 1)
bool init(size_t D, size_t WL, float Z, size_t bits_c);
#else
bool init(size_t D, size_t WL, size_t Z, size_t bits_c);
#endif
void inc(const unsigned char * str, size_t len, size_t delta);     //increase by one
#if (SF2 + SF3 + SF4 + SF5 + SF6 == 1)
void dec(const unsigned char * str, size_t len, size_t delta);     //decrease by one
#endif
size_t query(const unsigned char * str, size_t len);

#endif // SKETCH_H
