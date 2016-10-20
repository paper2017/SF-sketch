#ifndef CMLSKETCH_H
#define CMLSKETCH_H

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

class CMLSketch {
public:
	CMLSketch(uint d_, uint w_, uint counterBits_, double _b = 1.08, int _limit = 255);
	~CMLSketch();
	void insert(const uchar *str, uint len, uint r);
	// Query str's counter
	uint queryPoint(const uchar *str, uint len);

	bool decision(int c);
	double pointv(int c);
private:
	size_t **count;         // Memory for storing counters
	uint d, w;              // d rows, each of w counters
	uint counterBits;       // Memory bits for a single counter
	uint maxCounter;        // Upper limit for a counter
	BOBHash h[MAX_HF];      // Hash functions for each row

    int limit;
    double b;
    default_random_engine generator;
    uniform_real_distribution<double> distribution;
};

#endif
