#ifndef SKETCH_H
#define SKETCH_H
#include "sketch_config.h"
#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>

bool init(size_t D, size_t WL, size_t bits_c);

void inc(const unsigned char * str, size_t len, size_t delta);     //increase by one
#ifdef SUPPORT_DELETE
void dec(const unsigned char * str, size_t len, size_t delta);     //decrease by one
#endif
int64_t query(const unsigned char * str, size_t len);

#endif // SKETCH_H
