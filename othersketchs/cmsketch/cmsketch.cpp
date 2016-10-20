#include "cmsketch.h"

CountMinSketch::CountMinSketch(uint d_, uint w_, uint counterBits_) {
	if (counterBits_ > SZ_BITS) {
		printf("counterBits greater than %d not supported\n",
				SZ_BITS);   // Counter should not be bigger than a word
		exit(0);
	}
	d = d_;
	w = w_;
	counterBits = counterBits_;
	maxCounter = (1 << counterBits) - 1;
	// Allocate memory for sketch counters
	count = new size_t* [d];
	for (uint i = 0; i < d; ++i) {
        int len = w * counterBits / SZ_BITS + 1;
		count[i] = new size_t[len];
		memset(count[i], 0, len * sizeof(size_t));
		h[i].initialize(i); // Initialize hash functions
	}
}

CountMinSketch::~CountMinSketch() {
	for (uint i = 0; i < d; ++i)
		delete [] count[i];
	delete [] count;
}

// Get jth counter in row i. Each row of counters are treated
// as continuous bits represented by an array of words, so
// we have to calculate which word(s) the desired counter belongs
// to and fetch counter from the word(s). As counter is limited
// to no bigger than a word, there can be 1 or 2 words where
// the desired counter resides in.
inline uint CountMinSketch::getCounter(uint i, uint j) {
    // Desired counter bit range in row i: [lBit, hBit)
	uint lBit = j * counterBits;
	uint hBit = lBit + counterBits;
	// Which word lBit/(hBit-1) belongs to
	uint lIndex = SZ_ALIGN(lBit) >> SZ_ALIGN_BITS;
	uint hIndex = SZ_ALIGN(hBit - 1) >> SZ_ALIGN_BITS;
	// Conceptually, counter bits are stored in a form similar
	// to Big endian, eg. the most significant bit is lBit and
	// the least significant bit is hBit. Also, we think that
	// bits in each word are arranged in the same form. The
	// physical memory arrangement is probably quite not like that,
	// but it doesn't matter, as we will conform the same convention
	// in setCounter. Therefore, hgap means the bit gap between hBit
    // and the initial bit of next word.
	uint hgap = ((hIndex + 1) << SZ_ALIGN_BITS) - hBit;
	// If lIndex == hIndex, fetch the counter from the single word.
	// Otherwise, fetch corresponding bits from the two words
	// and merge them into a counter
	//
	// Case lIndex + 1 == hIndex:
	//     lIndex              hIndex
	//       |<----Lower Word---->|<----Higher Word--->|
	// ------|----------|---------|-|------------------|------
	//       |          |         | |                  |
	// ------|----------|---------|-|------------------|------
	//                  |<-lgap-->|-|<-----hgap------->|
	//                  |<-counter->|
	//                lBit         hBit
	//
	// Case lIndex == hIndex:
    // lIndex/hIndex
	//       |<-------Word------->|
	// ------|--|-----------|-----|---------------------------
	//       |  |           |     |
	// ------|--|-----------|-----|---------------------------
	//          |<-counter->| hgap|
	//        lBit         hBit
	//
	return lIndex == hIndex ?
			(count[i][hIndex] >> hgap) & maxCounter
		  : (((size_t)count[i][hIndex] >> hgap)
			+ (count[i][lIndex] << (SZ_BITS - hgap)))
			& maxCounter;
}

inline void CountMinSketch::setCounter(uint i, uint j, uint val) {
    // Conform the same convention in CountMinSketch::getCounter
    // lBit, hBit, lIndex, hIndex, hgap have the same meaning as before
	uint lBit = j * counterBits;
	uint hBit = lBit + counterBits;
	uint lIndex = SZ_ALIGN(lBit) >> SZ_ALIGN_BITS;
	uint hIndex = SZ_ALIGN(hBit - 1) >> SZ_ALIGN_BITS;
	uint hgap = ((hIndex + 1) << SZ_ALIGN_BITS) - hBit;
	// Set the corresponding bits in count[i][hIndex]
	count[i][hIndex] = count[i][hIndex] & ~(maxCounter << hgap) | (val << hgap);
	// If the counter resides in two words, set the corresponding
	// bits in lower word.
	if (lIndex < hIndex) {
        // Like hgap, lgap means the bit gap between lBits and the
        // initial bit of next word (counter[i][hIndex]).
		uint lgap = (hIndex << SZ_ALIGN_BITS) - lBit;
		count[i][lIndex] = count[i][lIndex] & (~0 << lgap)
							| (val >> (SZ_BITS - hgap));
	}
}

// Insert str and increase its counter by r
void CountMinSketch::insert(const uchar *str, uint len, uint r) {
	for (uint i = 0; i < d; ++i) {
		uint pos = h[i].run(str, len) % w;
		uint counter = getCounter(i, pos);
		counter = maxCounter - r > counter ? counter + r : maxCounter;
		setCounter(i, pos, counter);
	}
}


// Delete str and decrease its counter by r
void CountMinSketch::delete_(const uchar *str, uint len, uint r) {
	for (uint i = 0; i < d; ++i) {
		uint pos = h[i].run(str, len) % w;
		uint counter = getCounter(i, pos);
		counter = (r >= counter)? 0 : (counter - r);
		setCounter(i, pos, counter);
	}
}

// Query str's counter
uint CountMinSketch::queryPoint(const uchar *str, uint len) {
	uint ret = maxCounter;
	for (uint i = 0; i < d; ++i) {
		uint pos = h[i].run(str, len) % w;
		uint counter = getCounter(i, pos);
		ret = ret < counter ? ret : counter;
	}
	return ret;
}
