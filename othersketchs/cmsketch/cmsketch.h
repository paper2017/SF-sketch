#ifndef COUNTMINSKETCH_H
#define COUNTMINSKETCH_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>
#include "define.h"
#include "BOBHash.h"

//typedef uint size_t;        // A word; Basic memory unit for sketch

#define SZ_BITS 32          // Basic memory unit size for sketch
#define SZ_ALIGN_BITS 5     // Bits for align to size_t
// Align addr to multiple of SZ_BITS
#define SZ_ALIGN(addr) ((addr) & (~0 << SZ_ALIGN_BITS))

#define DECREASE_SUPPORT

using namespace std;

class CountMinSketch {
public:
	CountMinSketch(uint d_, uint w_, uint counterBits_);
	~CountMinSketch();
	// Get jth counter in row i
	uint getCounter(uint i, uint j);
	// Set jth counter in row i as val
	void setCounter(uint i, uint j, uint val);
	// Insert str and increase its counter by r
	void insert(const uchar *str, uint len, uint r);
#ifdef DECREASE_SUPPORT
    void delete_(const uchar *str, uint len, uint r);
#endif
	// Query str's counter
	uint queryPoint(const uchar *str, uint len);
private:
	size_t **count;         // Memory for storing counters
	uint d, w;              // d rows, each of w counters
	uint counterBits;       // Memory bits for a single counter
	uint maxCounter;        // Upper limit for a counter
	BOBHash h[MAX_HF];      // Hash functions for each row
};

#endif
