#include "cmlsketch.h"

CMLSketch::CMLSketch(uint d_, uint w_, uint counterBits_, double _b, int _limit)
{
    if (counterBits_ > SZ_BITS) {
        printf("counterBits greater than %d not supported\n",
               SZ_BITS);   // Counter should not be bigger than a word
        exit(0);
    }
    d = d_;
    w = w_;
    counterBits = counterBits_;
    b = _b;
    limit = _limit;
    maxCounter = (1 << counterBits) - 1;
    // Allocate memory for sketch counters
    count = new size_t* [d];
    for (uint i = 0; i < d; ++i) {
        count[i] = new size_t[w];
        memset(count[i], 0, sizeof(size_t) * w);
        h[i].initialize(i); // Initialize hash functions
    }
}

CMLSketch::~CMLSketch()
{
    for (uint i = 0; i < d; ++i)
        delete [] count[i];
    delete [] count;
}

bool CMLSketch::decision(int c)
{
    double r = distribution(generator);
    double lim = pow(b, -c);
    return r < lim;
}
double CMLSketch::pointv(int c)
{
    return c == 0 ? 0 : pow(b, c - 1);
}

// Insert str and increase its counter by r
void CMLSketch::insert(const uchar *str, uint len, uint r)
{
    uint pos;
    for (uint k = 0; k < r; ++k) {
        uint c = maxCounter;
        for (int i = 0; i < d; ++i) {
            pos = h[i].run(str, len) % w;
            if (count[i][pos] < c)
                c = count[i][pos];
        }
        if (c < limit && decision(c)) {
            for (int i = 0; i < d; ++i) {
                pos = h[i].run(str, len) % w;
                if (count[i][pos] == c)
                    ++count[i][pos];
            }
        }
    }
}

// Query str's counter
uint CMLSketch::queryPoint(const uchar *str, uint len)
{
    uint c = maxCounter;
    for (int i = 0; i < d; ++i) {
        uint pos = h[i].run(str, len) % w;
        if (count[i][pos] < c)
            c = count[i][pos];
    }
    return c <= 1 ? pointv(c) : (int)(round((1 - pointv(c + 1)) / (1 - b)));
}
