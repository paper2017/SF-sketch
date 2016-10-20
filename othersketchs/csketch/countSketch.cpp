#include <algorithm>
#include "string.h"
#include "BOBHash.h"
#include "sketch.h"
#include "sketch_config.h"

//const size_t MAX_BITS_C = 8 * sizeof(int64_t);
size_t dNum, wNum;
int64_t maxNum;
int64_t * s;

bool init(size_t D, size_t WL, size_t bits_c) {
    s = new int64_t [D * WL];
    memset(s, 0, D * WL * sizeof(int64_t));
    dNum = D;
    wNum = WL;
    if (bits_c > MAX_BITS_C) return false;
    maxNum = (bits_c >= MAX_BITS_C) ? INT64_MAX: ((int64_t)1 << bits_c) - 1;
    printf("maxNum: %ld, SIZE_MAX: %8lu\n", maxNum, SIZE_MAX);
    return true;
}

void inc(const unsigned char * str, size_t len, size_t delta) {
    for(uint32_t i = 0; i < dNum; ++i) {
        uint32_t val = hash(i, str, len) % wNum;
        int t = (hash2(i, str, len) & 1) ? 1 : -1;
//        int t = (val & 1) ? 1 : -1;
        int64_t * pos = s + i * wNum + val;
        int64_t tt = t * *pos;
        if (maxNum - (int64_t)delta < tt) {
            printf("F");
            *pos = maxNum * t;
        } else {
            *pos += (int64_t)delta * t;
        }
    }
}

void dec(const unsigned char * str, size_t len, size_t delta) {
    for(uint32_t i = 0; i < dNum; ++i) {
        uint32_t val = hash(i, str, len) % wNum;
        int t = (hash2(i, str, len) & 1) ? 1 : -1;
        int64_t * pos = s + i * wNum + val;
        int64_t tt = t * *pos;
        if (maxNum - (int64_t)delta < -tt) {
            *pos = -maxNum * t;
            printf("X");
        } else {
            *pos -= (int64_t)delta * t;
        }
    }
}


int64_t query(const unsigned char * str, size_t len) {
    int64_t * ss = new int64_t [dNum];
    for (int i = 0; i < dNum; ++i) {
        uint32_t val = hash(i, str, len) % wNum;
        int t = (hash2(i, str, len) & 1) ? 1 : -1;
        ss[i] = t * s[i * wNum + val];
    }
    size_t pos = dNum >> 1;
    std::nth_element(ss, ss + pos, ss + dNum);
    return ss[pos];
}
