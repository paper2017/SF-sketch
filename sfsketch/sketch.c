#include "sketch.h"
#include "memory.h"
#include <stdlib.h>

// get SSE 4.1 intrinsics
#include <smmintrin.h>
// get CPUID capability
//#include <intrin.h>

#ifdef CHECK_ABNORMAL
extern int vivi_counter;
#endif

static sfsketch sketch;
static size_t ALIGN_SHIFT;
//#ifdef SF_V1
#ifdef SF_SET_EXP_Z
bool init(size_t D, size_t WL, size_t exp_K, size_t bits_c) {
#else
bool init(size_t D, size_t WL, float f_K, size_t bits_c) {
#endif
    size_t K = 0;
    if (D > MAX_D || WL > MAX_WL ||
#ifdef SF_SET_EXP_Z
            exp_K > MAX_exp_K || exp_K <= 0 ||
#else
            f_K > MAX_K ||
#endif
            D == 0 || WL == 0 || bits_c > MAX_BITS_C) {
        printf("init: parameter/s out of range!\n");
        exit(-1);
        //return false;
    }
#ifdef SF_SET_EXP_Z
    K = 1 << exp_K;
    sketch.exp_K = exp_K;
#else
    K = f_K;
#endif

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
#ifdef SF_SET_DIVISIBLE_Z
    sketch.WR = (size_t)(K*WL);
#else
    sketch.WR = (size_t)(f_K*WL);
#endif
    sketch.K = K;
    sketch.bits_c = bits_c;
    sketch.max_c = (bits_c == MAX_BITS_C) ? SIZE_MAX : (((size_t)1 << bits_c) - 1);
//    printf("\nAAAAAAAAAAAAA\n\nsketch.K: %lu \n\n\n", sketch.K);

    size_t length = 0, total = 0;

//for right;
#ifdef COMPACT_COUNTER_R
    length = (sketch.WR * bits_c + (sizeof(size_t) * 8 - 1)) / (sizeof(size_t) * 8);
#else
    length = sketch.WR;
#endif
    total = sizeof(sketch_r) + sketch.D * (length * sizeof(size_t) + sizeof(void *));
    //if (!(sketch.right = (sketch_r *)aligned_alloc(32, total))) {
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
    //if (!(sketch.left = (sketch_l *)aligned_alloc(32, total))) {
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

#ifdef SF_SET_DIVISIBLE_Z
static inline void get_k_w(size_t random, size_t *pk, size_t *pw) { //random = (k, w)
#ifdef SF_SET_EXP_Z
    *pk = random & ((1 << sketch.exp_K) - 1);
    *pw = random >> sketch.exp_K;
#else
    *pk = random % sketch.K;
    *pw = random / sketch.K;
#endif
}
#endif



#ifdef SF_AGING
static void inc_r_only(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, random, counter;
    for (d = 0; d < sketch.D; ++d) {
        random = hash(d, str, len) % sketch.WR;
        counter = get_rc(d, random);
        counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
        set_rc(d, random, counter);
    }
}
#else
static void inc_r(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, random, counter;
//----------------------------------
//    static int atest = 0;
//----------------------------------
#ifdef SF_SET_MIN_COUNTER
    size_t min = sketch.max_c;
#endif
    for (d = 0; d < sketch.D; ++d) {
        random = hash(d, str, len) % sketch.WR;
#ifdef SF_SET_DIVISIBLE_Z
        size_t k, w;
        get_k_w(random, &k, &w);
        sketch.auxis[d].w = w;
#endif
#ifdef SF_SET_MAX_COUNTERS_ARRAY
        size_t max = 0;
        for (int i = 0; i < sketch.K; i++) {
            counter = get_rc(d, sketch.auxis[d].w * sketch.K + i);

            if (i == k) {
                //TODO  是否考虑计数器为负数的情况？！！！！
                counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
                set_rc(d, sketch.auxis[d].w * sketch.K + i, counter);
#ifdef SF_SET_MIN_COUNTER
                if (min > counter) {
                    min = counter;
                }
#endif
            }
            if (max < counter) {
                max = counter;
            }
        }
        sketch.auxis[d].c = max;
#else
        counter = get_rc(d, random);
        counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
        set_rc(d, random, counter);
#ifdef SF_SET_MIN_COUNTER
        if (min > counter) {
            min = counter;
        }
#endif
#endif
//----------------------------------
        /*
                atest++;
                if (atest > 500000 && atest < 500010) {
                    printf("\nw=%ld, wtmp=%ld, k=%ld, delta=%ld, counter=%ld\n", w, wtmp, k, delta, counter);
                fflush(stdout);

                }
        */
//----------------------------------
    }
#ifdef SF_SET_MIN_COUNTER
    sketch.aux_minr = min;
#endif
}
#ifdef SF_SET_DIVISIBLE_Z 
static void inc_l(size_t delta) {
#else
static void inc_l(const unsigned char * str, size_t len, size_t delta) {
#endif
    size_t d = 0;
#ifdef SF_SET_MAX_COUNTERS_ARRAY
    size_t counter;
    for (d = 0; d < sketch.left->D; ++d) {
        counter = get_lc(d, sketch.auxis[d].w);
        assert(counter <= sketch.auxis[d].c);
        if (counter < sketch.auxis[d].c) {
            set_lc(d, sketch.auxis[d].w, sketch.auxis[d].c);
        }
    }
#else
    size_t minl = sketch.max_c, minr = sketch.aux_minr;
    for (d = 0; d < sketch.left->D; ++d) {
#ifdef SF_SET_LEFT_HASHS
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
#endif
    return;
}
#endif

void inc(const unsigned char * str, size_t len, size_t delta) {
#ifdef SF_AGING
    inc_r_only(str, len, delta);
#else
    inc_r(str, len, delta);
#ifdef SF_SET_DIVISIBLE_Z
    inc_l(delta);
#else
    inc_l(str, len, delta);
#endif
#endif
}

#ifdef SF_SET_SUPPORT_DELETE 
#ifdef SF_AGING
static void dec_r_only(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, random, counter;
    for (d = 0; d < sketch.D; ++d) {
        random = hash(d, str, len) % sketch.WR;
        counter = get_rc(d, random);
        counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
        set_rc(d, random, counter);
    }
}
#else
static void dec_r(const unsigned char * str, size_t len, size_t delta) {
    size_t d = 0, random, w, counter;
//----------------------------------
//    static int atest = 0;
//----------------------------------
    for (d = 0; d < sketch.D; ++d) {
        random = hash(d, str, len) % sketch.WR;
        size_t k;
        get_k_w(random, &k, &w);
        sketch.auxis[d].w = w;
        size_t max = 0;
        for (int i = 0; i < sketch.K; i++) {
            counter = get_rc(d, sketch.auxis[d].w * sketch.K + i);

            if (i == k) {
                //TODO  是否考虑计数器为负数的情况？！！！！
                //counter = (((sketch.right->max_c - delta) > counter) ? (counter + delta) : sketch.right->max_c);
                (counter > delta) ? (counter -= delta) : (counter = 0);
                set_rc(d, sketch.auxis[d].w * sketch.K + i, counter);
            }
            if (max < counter) {
                max = counter;
            }
        }
        sketch.auxis[d].c = max;
    }
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
#endif
void dec(const unsigned char * str, size_t len, size_t delta) {
#ifdef SF_AGING
    dec_r_only(str, len, delta);
#else
    dec_r(str, len, delta);
    dec_l(delta);
#endif
}
#endif

#if (SF_AGING == 1)
//TEST BIG END OR LITTLE ENDING
//static union { char c[4]; unsigned long mylong; } endian_test = {{ 'l', '?', '?', 'b' } };
//#define ENDIANNESS ((char)endian_test.mylong)

void compose_slim(void) {
    size_t d, w, max, counter;
    //printf("sketch.bits_c = %lu ; sizeof = %lu; ENDING %c\n", sketch.bits_c, sizeof(size_t), ENDIANNESS);
    __m128i ymm0, ymm1, ymm2;
    if (sketch.bits_c == 32 && sketch.K == 16) {//right now, only for K == 16
        //__attribute__((aligned(16)))
        uint32_t arry[16][4];// __attribute__((aligned(32)));
        /*  just for test!!!!
        uint32_t arry4[2][4]={{2,1,4,3},{0}};
        size_t *fuck[2] = {(size_t *)arry4[0], (size_t *)arry4[1]};
        ymm2 = _mm_loadu_si128((__m128i const *) arry4[0]);
        _mm_storeu_si128((__m128i *)arry4[1], ymm2);
        for(int i=0; i<4;i++){
        	printf("arry04[%i] = %u | %lu ,\n", i, ((uint32_t*)arry4[1])[i], get_cpt_c(1, i, fuck));
        }
        */

        for(d = 0; d < sketch.D; ++d) {
            int b = 0;
            uint32_t * pright = (uint32_t *)&sketch.right->counts[d][0];
            uint32_t * pleft = (uint32_t *)&sketch.left->counts[d][0];
//sketch.WL
            for(w=0; w< sketch.WL; w+=4, b = w << 4) {
                for(int i=0; i<16; i++, b++) {
                    //to compare with get_cpt_c which is a BIG ENDING , while LINUX is Little ENDING
                    arry[i][1] = pright[b];
                    arry[i][0] = pright[b+16];
                    arry[i][3] = pright[b+32];
                    arry[i][2] = pright[b+48];
                }
                ymm0 = _mm_loadu_si128((__m128i const *) arry[0]);
                for(int i=0; i<14; i+=2) {
                    ymm2 = _mm_loadu_si128((__m128i const *) arry[i+1]);
                    ymm1 = _mm_max_epu32(ymm0, ymm2);
                    ymm2 = _mm_loadu_si128((__m128i const *) arry[i+2]);
                    ymm0 = _mm_max_epu32(ymm1, ymm2);
                }
                ymm2 = _mm_loadu_si128((__m128i const *) arry[15]);
                ymm1 = _mm_max_epu32(ymm0, ymm2);
                _mm_storeu_si128((__m128i *)(pleft+w), ymm1);


                /* for debug
                for(int i=0; i<4; i++){
                	for(int j=0; j<16; j++){
                		printf("[%i][%i] = %u | %lu; ", i, j, arry[j][i], get_rc(d, w * sketch.K + i*16+j));
                	}
                	printf("pleft[%i] = %u| %lu; \n", i, pleft[w+i], get_lc(d, w + i));
                }
                */

            }
        }
    } else {
        for (d = 0; d < sketch.D; ++d) {
            for (w = 0; w < sketch.WL; w++) {
                max = 0;
                for (int i = 0; i < sketch.K; i++) {
                    counter = get_rc(d, w * sketch.K + i);
                    if (max < counter) {
                        max = counter;
                    }
                }
                set_lc(d, w, max);
            }
        }
    }
}

size_t query_fat(const unsigned char * str, size_t len) {
    size_t d, w, min, counter;
    w = hash(0, str, len) % sketch.WR;
    //TODO 可以改进一下！
    //one split:
#ifndef FAST_VX
    size_t k = w / sketch.WL;
    w = w % sketch.WL;
    w = w * sketch.K + k;
#endif

    min = get_rc(0, w);

    for (d = 1; d < sketch.D; ++d) {
        w = hash(d, str, len) % sketch.WR;
#ifndef FAST_VX
        k = w / sketch.WL;
        w = w % sketch.WL;
        w = w * sketch.K + k;
#endif
        counter = get_rc(d, w);

        if (min > counter) {
            min = counter;
#ifdef CHECK_ABNORMAL
            vivi_counter++;
#endif
        }
        //min = min < counter ? min : counter;
    }
    return min;
}

#endif


size_t query_slim(const unsigned char * str, size_t len) {
    size_t d, w, min, counter;
#ifdef SF_SET_LEFT_HASHS
        w = hash_l(0, str, len) % sketch.left->W;
#else
    w = hash(0, str, len) % sketch.WR;
#ifndef FAST_VX
    w = w / sketch.K;
#else
    //w = w / sketch.K;
    w = w >> sketch.exp_K; //assuming K = 2^3 = 8;
#endif
#endif
    min = get_lc(0, w);

    for (d = 1; d < sketch.D; ++d) {
#ifdef SF_SET_LEFT_HASHS
        w = hash_l(d, str, len) % sketch.left->W;
#else
        w = hash(d, str, len) % sketch.WR;
#ifndef FAST_VX
        w = w / sketch.K;
#else
        //w = w / sketch.K;
        w = w >> sketch.exp_K;
#endif
#endif
        counter = get_lc(d, w);

        if (min > counter) {
            min = counter;
#ifdef CHECK_ABNORMAL
            vivi_counter++;
#endif
        }
        //min = min < counter ? min : counter;
    }
    return min;
}

#ifndef SF_AGING
size_t query(const unsigned char * str, size_t len) {
    return query_slim(str, len);
}
#else
static sketch_name query_sketch;

void set_query_slim(void) {
    query_sketch = LEFT_SKETCH;
}

void set_query_fat(void) {
    query_sketch = RIGHT_SKETCH;
}

size_t query(const unsigned char * str, size_t len) {
    return query_sketch == LEFT_SKETCH ? query_slim(str, len) : query_fat(str, len);
}
#endif

float calc_loadfactor(void) {
    size_t nzeros = 0;
    for(int i=0; i<sketch.D; i++) {
        for(int j=0; j<sketch.WL; j++) {
            if (get_lc(i, j)== 0)
                nzeros++;
        }
    }
    return 100.0 - 100.0 * nzeros / (sketch.WL * sketch.D);
}
