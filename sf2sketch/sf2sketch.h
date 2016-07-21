#ifndef SF2_SKETCH_H
#define SF2_SKETCH_H

#include "../sketch_config.h"
#ifdef SF2
typedef sketch_l sketch_r;
typedef sketch_l sketch_r2;

struct auxi_t {
    size_t w;
    size_t c;
};

typedef struct sf2sketch_t {
    size_t D;
    size_t WL;
    size_t WR;
    size_t Z;

    //counter:
	size_t bits_c;       // Memory bits for a single counter
	size_t max_c;        // Upper limit for a counter

	//sketch:
	sketch_l * left;     //slim sketch
	sketch_r * right;    //fat sketch
	sketch_r2 * right2;  //delection sketch

	//auxilary data
    struct auxi_t auxis[MAX_D];
    size_t aux_minr;
}sf2sketch;
#endif // SF2
#endif // SF2_SKETCH_H
