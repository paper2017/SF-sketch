#include "sketch.h"
#include "memory.h"
#include <stdlib.h>
static fcsketch sketch;
static size_t ALIGN_SHIFT;
#ifdef FC_V1
bool init(size_t D, size_t WL, float K, size_t bits_c) {
#else
bool init(size_t D, size_t WL, size_t K, size_t bits_c) {
#endif
    if (D > MAX_D || WL > MAX_WL || K > MAX_K ||
        D == 0 || WL == 0 || K <= 0 || bits_c > MAX_BITS_C) {
        dbg_printf("init: parameter/s out of range!");
        return false;
    }

    if (MAX_BITS_C == 32)
        ALIGN_SHIFT = 5;
    else if (MAX_BITS_C == 64)
        ALIGN_SHIFT = 6;
    else if (MAX_BITS_C == 16)
        ALIGN_SHIFT = 4;
    else
    {
        dbg_printf("unrecognized machine!");
        return false;
    }


    sketch.D = D;
    sketch.WL = WL;
    sketch.WR = (size_t)(K*WL);
#ifdef FC_V1
    if (K*WL > sketch.WR) {
        sketch.WR++;
    }
#endif
    sketch.K = K;
    sketch.bits_c = bits_c;
    sketch.max_c = (bits_c == MAX_BITS_C) ? SIZE_MAX : (((size_t)1 << bits_c) - 1);

    size_t length = 0, total = 0;

//for right;
#ifdef COMPACT_COUNTER_R
    length = (sketch.WR * bits_c + (sizeof(size_t) * 8 - 1)) / (sizeof(size_t) * 8);
#else
    length = sketch.WR;
#endif
    total = sizeof(sketch_r) + sketch.D * (length * sizeof(size_t) + sizeof(void *));
    if (!(sketch.right = (sketch_r *)malloc(total))) {
        dbg_printf("init: malloc failed!");
        return false;
    }
    memset(sketch.right, 0, total);
    sketch.right->D = D;
    sketch.right->W = sketch.WR;
    sketch.right->bits_c = bits_c;
    sketch.right->max_c = sketch.max_c;
    sketch.right->length = length;
    for (int i=0; i<sketch.D; i++) {
        sketch.right->counts[i] = (size_t *)(&sketch.right->counts[sketch.D]) + i * length;
    }

//for left:
#ifdef COMPACT_COUNTER_L
    length = (WL * bits_c + (sizeof(size_t) * 8 - 1)) / (sizeof(size_t) * 8);
#else
    length = WL;
#endif
    total = sizeof(sketch_l) + sketch.D * (length * sizeof(size_t) + sizeof(void *));
    if (!(sketch.left = (sketch_l *)malloc(total))) {
        dbg_printf("init: malloc failed!");
        return false;
    }
    memset(sketch.left, 0, total);
    sketch.left->D = D;
    sketch.left->W = WL;
    sketch.left->bits_c = bits_c;
    sketch.left->max_c = sketch.max_c;
    sketch.left->length = length;
    for (int i=0; i<sketch.D; i++) {
        sketch.left->counts[i] = (size_t *)(&sketch.left->counts[sketch.D]) + i * length;
    }
    return true;
}
#if (COMPACT_COUNTER_L + COMPACT_COUNTER_R != 0 )
static size_t get_cpt_c(size_t d, size_t w, size_t * counts[]) {
        // Desired counter bit range in row i: [lBit, hBit)
    size_t lBit = w * sketch.bits_c;
    size_t hBit = lBit + sketch.bits_c;
    // Which word lBit/(hBit-1) belongs to
    size_t lIndex = SZ_ALIGN(lBit) >> ALIGN_SHIFT;//#define SZ_ALIGN(addr) ((addr) & (~0 << ALIGN_SHIFT))
    size_t hIndex = SZ_ALIGN(hBit - 1) >> ALIGN_SHIFT;
    // Conceptually, counter bits are stored in a form similar
    // to Big endian, eg. the most significant bit is lBit and
    // the least significant bit is hBit. Also, we think that
    // bits in each word are arranged in the same form. The
    // physical memory arrangement is probably quite not like that,
    // but it doesn't matter, as we will conform the same convention
    // in setCounter. Therefore, hgap means the bit gap between hBit
    // and the initial bit of next word.
    size_t hgap = ((hIndex + 1) << ALIGN_SHIFT) - hBit;
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
            ((counts[d][hIndex] >> hgap) & sketch.max_c)
          : (((counts[d][hIndex] >> hgap)
            + (counts[d][lIndex] << (MAX_BITS_C - hgap))) & sketch.max_c);
}
#endif
static size_t get_rc(size_t d, size_t w) {
#ifdef COMPACT_COUNTER_R
    return get_cpt_c(d, w, sketch.right->counts);
#else
    return sketch.right->counts[d][w];
#endif
}

static size_t get_lc(size_t d, size_t w) {
#ifdef COMPACT_COUNTER_L
    return get_cpt_c(d, w, sketch.left->counts);
#else
    return sketch.left->counts[d][w];
#endif
}


#if (COMPACT_COUNTER_L + COMPACT_COUNTER_R != 0 )
static void set_cpt_c(size_t d, size_t w, size_t val, size_t * counts[]) {
    size_t lBit = w * sketch.bits_c;
    size_t hBit = lBit + sketch.bits_c;
    size_t lIndex = SZ_ALIGN(lBit) >> ALIGN_SHIFT;
    size_t hIndex = SZ_ALIGN(hBit - 1) >> ALIGN_SHIFT;
    size_t hgap = ((hIndex + 1) << ALIGN_SHIFT) - hBit;
	// Set the corresponding bits in count[i][hIndex]
    counts[d][hIndex] = (counts[d][hIndex] & ~(sketch.max_c << hgap)) | (val << hgap);    
	// If the counter resides in two words, set the corresponding
	// bits in lower word.
	if (lIndex < hIndex) {
        // Like hgap, lgap means the bit gap between lBits and the
        // initial bit of next word (counter[i][hIndex]).
		size_t lgap = (hIndex << ALIGN_SHIFT) - lBit;
		counts[d][lIndex] = (counts[d][lIndex] & (~0 << lgap)) | (val >> (MAX_BITS_C - hgap));
	}
}
#endif

static void set_rc(size_t d, size_t w, size_t val) {
#ifdef COMPACT_COUNTER_R
    set_cpt_c(d, w, val, sketch.right->counts);
#else
    sketch.right->counts[d][w] = val;
#endif
}

static void set_lc(size_t d, size_t w, size_t val) {
#ifdef COMPACT_COUNTER_L
    set_cpt_c(d, w, val, sketch.left->counts);
#else
    sketch.left->counts[d][w] = val;
#endif
}

size_t query_counter(size_t d, size_t w, sketch_name name) {
    if (name == LEFT_SKETCH) {
        if (d >= sketch.left->D || w >= sketch.left->W) {
            printf("unexpected access domain");
            return 0; //»¹ÊÇÓÐÐ©ÎÊÌâ¡«·µ»Ø-1Ò²²»ÀíÏë¡£
        } else {
            return get_lc(d, w);
        }
    } else if (name == RIGHT_SKETCH) {
        if (d >= sketch.right->D || w >= sketch.right->W) {
            printf("unexpected access domain");
            return 0;
        }
        return get_rc(d, w);
    } else {
        printf("unexpected sketch name");
        return 0;
    }
}

static void inc_r(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, w, counter;
//----------------------------------
//    static int atest = 0;
//----------------------------------
#ifndef FC_V3
    size_t min = sketch.max_c;
#endif
	for (d = 0; d < sketch.D; ++d) {
		w = hash(d, str, len) % sketch.WR;

#ifndef FC_V3
#ifdef FC_V4
        //TODO 可以改进一下！
		size_t k = w / sketch.WL;
		w = w % sketch.WL;
//----------------------------------
//        size_t wtmp = w;
//----------------------------------
        sketch.auxis[d].w = w;
        w = w * sketch.K + k;
#endif
		counter = get_rc(d, w);
//----------------------------------
/*
        atest++;
        if (atest > 500000 && atest < 500010) {
            printf("\nw=%ld, wtmp=%ld, k=%ld, delta=%ld, counter=%ld\n", w, wtmp, k, delta, counter);
        fflush(stdout);

        }
*/
//----------------------------------
        counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
        set_rc(d, w, counter);

		if (min > counter) {
		    min = counter;
		}
#ifdef FC_V2
        w = w % sketch.WL;
        sketch.auxis[d].w = w;
#endif //FC_V2

#else //FC_V3
        //TODO 可以改进一下！
		size_t k = w / sketch.WL;
		w = w % sketch.WL;
        sketch.auxis[d].w = w;

        size_t max = 0;
        for (int i = 0; i < sketch.K; i++) {
            counter = get_rc(d, w * sketch.K + i);

            if (i == k) {
                //TODO  是否考虑计数器为负数的情况？！！！！
                counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
                set_rc(d, w * sketch.K + i, counter);
            }
            if (max < counter) {
                max = counter;
            }
        }
        sketch.auxis[d].c = max;
#endif //FC_V3
	}
#ifndef FC_V3
    sketch.aux_minr = min;
#endif
}

#ifdef FC_V1
static void inc_l(const unsigned char * str, size_t len, size_t delta) {
#else
static void inc_l(size_t delta) {
#endif
    size_t d = 0;
#ifndef FC_V3
    size_t minl = sketch.max_c, minr = sketch.aux_minr;
	for (d = 0; d < sketch.left->D; ++d) {
#ifdef FC_V1
		sketch.auxis[d].w = hash_l(d, str, len) % sketch.left->W;
#endif
		sketch.auxis[d].c = get_lc(d, sketch.auxis[d].w);
		if (minl > sketch.auxis[d].c) {
		    minl = sketch.auxis[d].c;
		}
	}

	if (minl >= minr) return;
	((minr - delta) <= minl) ? (minl = minr) : (minl += delta);

	for (d = 0; d < sketch.left->D; ++d) {
		if (sketch.auxis[d].c < minl) {
            set_lc(d, sketch.auxis[d].w, minl);
		}
	}
#else //FC_V3
    size_t counter;
    for (d = 0; d < sketch.left->D; ++d) {
        counter = get_lc(d, sketch.auxis[d].w);
        assert(counter <= sketch.auxis[d].c);
        if (counter < sketch.auxis[d].c) {
            set_lc(d, sketch.auxis[d].w, sketch.auxis[d].c);
        }
    }
#endif//FC_V3
	return;
}

void inc(const unsigned char * str, size_t len, size_t delta) {
    inc_r(str, len, delta);
#ifdef FC_V1
    inc_l(str, len, delta);
#else
    inc_l(delta);
#endif
}

#if (FC_V2 + FC_V3 + FC_V4 == 1)
static void dec_r(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, max = 0, k = 0, w, i, counter;
	for (d = 0; d < sketch.D; ++d) {
        max = 0;
		w = hash(d, str, len) % sketch.WR;
		k = w / sketch.WL;
		w = w % sketch.WL;
        sketch.auxis[d].w = w;
        for (i = 0; i < sketch.K; i++) {
#ifdef FC_V2
            counter = get_rc(d, sketch.WL * i + w);
#else
            counter = get_rc(d, w * sketch.K + i);
#endif
            //对第三版本以后的，我们可以再改进一下
            if (i == k) {
                //TODO  是否考虑计数器为负数的情况？！！！！
                (counter > delta) ? (counter -= delta) : (counter = 0);
                //counter = ((counter > delta) ? (counter - delta) : 0);
#ifdef FC_V2
                set_rc(d, sketch.WL * i + w, counter);
#else
                set_rc(d, w * sketch.K + i, counter);
#endif
            }
            if (max < counter) {
                max = counter;
            }
        }
        sketch.auxis[d].c = max;
	}
	return;
}
static void dec_l(size_t delta) {
    size_t d = 0, counter;
    for (d = 0; d < sketch.D; d++) {
        counter = get_lc(d, sketch.auxis[d].w);
        assert(counter <= (sketch.auxis[d].c + delta));
        if (counter > sketch.auxis[d].c) {
            set_lc(d, sketch.auxis[d].w, sketch.auxis[d].c);
        }
    }
}
void dec(const unsigned char * str, size_t len, size_t delta) {
    dec_r(str, len, delta);
    dec_l(delta);
}
#endif //(FC_V2 + FC_V3 + FC_V4 == 1)

size_t query(const unsigned char * str, size_t len) {
    size_t d = 0, w, min = sketch.max_c, counter;
	for (d = 0; d < sketch.D; ++d) {
		w = hash(d, str, len) % sketch.WL;
		counter = get_lc(d, w);
		if (min > counter) {
		    min = counter;
		}
	}
	return min;
}
