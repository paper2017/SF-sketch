#include "sf3sketch.h"
#ifdef SF3
#include "memory.h"
#include <stdlib.h>
static sf3sketch sketch;
static size_t ALIGN_SHIFT;
bool init(size_t D, size_t WL, size_t Z, size_t bits_c)
{
    if (D > MAX_D || WL > MAX_WL || Z > MAX_Z ||
        D == 0 || WL == 0 || Z <= 0 || bits_c > MAX_BITS_C) {
        dbg_printf("init: parameter/s out of range!");
        return false;
    }

    if (MAX_BITS_C == 32)
        ALIGN_SHIFT = 5;
    else if (MAX_BITS_C == 64)
        ALIGN_SHIFT = 6;
    else if (MAX_BITS_C == 16)
        ALIGN_SHIFT = 4;
    else {
        dbg_printf("unrecognized machine!");
        return false;
    }


    sketch.D = D;
    sketch.WL = WL;
    sketch.WR = (size_t)(Z*WL);
    sketch.Z = Z;
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
static size_t get_cpt_c(size_t d, size_t w, size_t * counts[])
{
    size_t lBit = w * sketch.bits_c;
    size_t hBit = lBit + sketch.bits_c;
    size_t lIndex = SZ_ALIGN(lBit) >> ALIGN_SHIFT;
    size_t hIndex = SZ_ALIGN(hBit - 1) >> ALIGN_SHIFT;
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
static size_t get_rc(size_t d, size_t w)
{
#ifdef COMPACT_COUNTER_R
    return get_cpt_c(d, w, sketch.right->counts);
#else
    return sketch.right->counts[d][w];
#endif
}

static size_t get_lc(size_t d, size_t w)
{
#ifdef COMPACT_COUNTER_L
    return get_cpt_c(d, w, sketch.left->counts);
#else
    return sketch.left->counts[d][w];
#endif
}


#if (COMPACT_COUNTER_L + COMPACT_COUNTER_R != 0 )
static void set_cpt_c(size_t d, size_t w, size_t val, size_t * counts[])
{
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

static void set_rc(size_t d, size_t w, size_t val)
{
#ifdef COMPACT_COUNTER_R
    set_cpt_c(d, w, val, sketch.right->counts);
#else
    sketch.right->counts[d][w] = val;
#endif
}

static void set_lc(size_t d, size_t w, size_t val)
{
#ifdef COMPACT_COUNTER_L
    set_cpt_c(d, w, val, sketch.left->counts);
#else
    sketch.left->counts[d][w] = val;
#endif
}

static void inc_r(const unsigned char * str, size_t len, size_t delta)
{
    size_t d = 0, w, counter;
    size_t min = sketch.max_c;
    for (d = 0; d < sketch.D; ++d) {
        w = hash(d, str, len) % sketch.WR;
        counter = get_rc(d, w);
        counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
        set_rc(d, w, counter);

        if (min > counter) {
            min = counter;
        }
        sketch.auxis[d].w = w % sketch.WL;
    }
    sketch.aux_minr = min;
}

static void inc_l(size_t delta)
{
    size_t d = 0;
    size_t minl = sketch.max_c, minr = sketch.aux_minr;
    for (d = 0; d < sketch.left->D; ++d) {
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
    return;
}

void inc(const unsigned char * str, size_t len, size_t delta)
{
    inc_r(str, len, delta);
    inc_l(delta);
}

static void dec_r(const unsigned char * str, size_t len, size_t delta)
{
    size_t d = 0, sum = 0, z = 0, w, i, counter;
    for (d = 0; d < sketch.D; ++d) {
        sum = 0;
        w = hash(d, str, len) % sketch.WR;
        z = w / sketch.WL;
        w = w % sketch.WL;
        sketch.auxis[d].w = w;
        for (i = 0; i < sketch.Z; i++) {
            counter = get_rc(d, sketch.WL * i + w);
            if (i == z) {
                (counter > delta) ? (counter -= delta) : (counter = 0);
                set_rc(d, sketch.WL * i + w, counter);
            }
            sum += counter;
        }
        sketch.auxis[d].c = sum;
    }
    return;
}
static void dec_l(size_t delta)
{
    size_t d = 0, counter;
    for (d = 0; d < sketch.D; d++) {
        counter = get_lc(d, sketch.auxis[d].w);
        assert(counter <= (sketch.auxis[d].c + delta));
        if (counter > sketch.auxis[d].c) {
            set_lc(d, sketch.auxis[d].w, sketch.auxis[d].c);
        }
    }
}
void dec(const unsigned char * str, size_t len, size_t delta)
{
    dec_r(str, len, delta);
    dec_l(delta);
}

size_t query(const unsigned char * str, size_t len)
{
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
#endif // SF3
