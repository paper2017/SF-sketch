#ifndef CMCUSKETCH_H
#define CMCUSKETCH_H

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>
#include "define.h"
#include "BOBHash.h"

//typedef uint size_t;        // A word; Basic memory unit for sketch

#define SZ_BITS (8*sizeof(size_t))          // Basic memory unit size for sketch
#define SZ_ALIGN_BITS 5     // Bits for align to size_t
// Align addr to multiple of SZ_BITS
#define SZ_ALIGN(addr) ((addr) & (~0 << SZ_ALIGN_BITS))

using namespace std;

struct auxi_t {
    uint w;
    uint c;
};

class CMCUSketch {
public:
	CMCUSketch(uint d_, uint w_, uint counterBits_);
	~CMCUSketch();
	// Get jth counter in row i
	uint getCounter(uint i, uint j);
	// Set jth counter in row i as val
	void setCounter(uint i, uint j, uint val);
	// Insert str and increase its counter by r
	void insert(const uchar *str, uint len, uint r);
	// Query str's counter
	uint queryPoint(const uchar *str, uint len);
private:
	size_t **count;         // Memory for storing counters
	uint d, w;              // d rows, each of w counters
	uint counterBits;       // Memory bits for a single counter
	uint maxCounter;        // Upper limit for a counter
	BOBHash h[MAX_HF];      // Hash functions for each row
    struct auxi_t auxis[MAX_D];
};

#endif
