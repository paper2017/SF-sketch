#ifndef COUNTMINSKETCH_H
#define COUNTMINSKETCH_H

#include "define.h"
#include <iostream>
#include "BOBHash.h"
#include <algorithm>
using namespace std;

#define MAX_HF 20

/*
bf_base初始化时估计大概的空间
*/

class CountMinSketch
{
public:
	CountMinSketch();
	~CountMinSketch();

	CountMinSketch(uint m, uint k, uint counterBits);

	int insert(const unsigned char * str, unsigned int len, unsigned int r); //Insert the same element r times
	uint query(const unsigned char * str, unsigned int len);

	uint getMemAccNum() {
		return this->querymemAccNum;
	}

private:
	uint getCounterFromOneByte(uint ki, uint start_byte, uint start_bit_pos);
	uint getCounterFromTwoBytes(uint ki, uint start_byte, uint start_bit_pos, uint end_byte, uint end_bit_pos);

	uchar bf_base[18][28000]; //bloom filter base
	uint bf_m; //bloom filter length, # of counter in each row
	uint bf_k; //hash function numbers;
	uint bf_n; //# of elements inserted
	uint counterBits;
	uint max_counter;

	BOBHash bf_hfp[MAX_HF];

	uint querymemAccNum;
};

#endif