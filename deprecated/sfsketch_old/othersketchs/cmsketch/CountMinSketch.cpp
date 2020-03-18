#include "CountMinSketch.h"


CountMinSketch::CountMinSketch()
{
}


CountMinSketch::~CountMinSketch()
{
}

CountMinSketch::CountMinSketch(uint m, uint k, uint counterBits) {

	if (counterBits > 9) {
		cerr << "Not support counter bits greater than 9" << endl;
	}

	this->bf_k = k;
	this->bf_m = m;
	this->counterBits = counterBits;

	this->max_counter = (1 << counterBits) - 1;
	this->bf_n = 0;
	querymemAccNum = 0;

	memset(bf_base, 0, sizeof(bf_base));
	for (int i = 0; i < bf_k; i++)
		bf_hfp[i].initialize(i);
}

uint CountMinSketch::getCounterFromOneByte(uint ki, uint start_byte, uint start_bit_pos) {
	return ((uchar)(bf_base[ki][start_byte] << (start_bit_pos % 8)) >> (8 - counterBits));
}

uint CountMinSketch::getCounterFromTwoBytes(uint ki, uint start_byte, uint start_bit_pos, uint end_byte, uint end_bit_pos) {
	uint start_le = start_bit_pos % 8;
	uint end_le = end_bit_pos % 8 + 1;
	uint end_ri = 8 - end_le;
	return (((uchar)(bf_base[ki][start_byte] << (start_le)) >> start_le) << end_le) | (bf_base[ki][end_byte] >> end_ri);
}

int CountMinSketch::insert(const unsigned char * str, uint len, uint r) { //Insert the same element r times
	if (r <= 0) return 1;

	for (uint i = 0; i < bf_k; i++){
		uint pos = bf_hfp[i].run(str, len) % bf_m;
		uint start_bit_pos = pos * counterBits;
		uint end_bit_pos = (1 + pos) * counterBits - 1;
		uint start_byte = start_bit_pos / 8;
		uint end_byte = end_bit_pos / 8;

		if (start_byte == end_byte) {
			uint counter = getCounterFromOneByte(i, start_byte, start_bit_pos);
			counter = min(counter + r, this->max_counter);

			uint t2 = 8 - (start_bit_pos % 8);
			uint t3 = 7 - (end_bit_pos % 8);
			uint t4 = 8 - t3;
			bf_base[i][start_byte] = ((bf_base[i][start_byte] >> t2) << t2) | (counter << t3) | ((uchar)(bf_base[i][start_byte] << t4) >> t4); //Type casting!!!

		}
		else {//start_byte + 1 == end_byte
			uint counter = getCounterFromTwoBytes(i, start_byte, start_bit_pos, end_byte, end_bit_pos);
			counter = min(counter + r, this->max_counter);

			uint start_ri = 8 - (start_bit_pos % 8);
			uint end_le = end_bit_pos % 8 + 1;
			uint end_ri = 8 - end_le;
			bf_base[i][start_byte] = ((bf_base[i][start_byte] >> start_ri) << start_ri) | (counter >> (end_le));
			uint t2 = counter - ((counter >> end_le) << end_le);
			bf_base[i][end_byte] = (t2 << end_ri) | ((uchar)(bf_base[i][end_byte] << end_le) >> end_le);

		}
	}

	bf_n++;
	return 1;
}

uint CountMinSketch::query(const unsigned char * str, unsigned int len) {
	uint minCounter = 1000000;

	for (uint i = 0; i < bf_k; i++){
		uint pos = bf_hfp[i].run(str, len) % bf_m;

		uint start_bit_pos = pos * counterBits;
		uint end_bit_pos = (1 + pos) * counterBits - 1;
		uint start_byte = start_bit_pos / 8;
		uint end_byte = end_bit_pos / 8;

		uint counter;
		if (start_byte == end_byte) {
			counter = getCounterFromOneByte(i, start_byte, start_bit_pos);
		}
		else {//start_byte + 1 == end_byte
			counter = getCounterFromTwoBytes(i, start_byte, start_bit_pos, end_byte, end_bit_pos);
		}
		this->querymemAccNum++;
		if (counter == 0) return 0;
		if (counter < minCounter) {
			minCounter = counter;
		}
	}
	return minCounter;
}
