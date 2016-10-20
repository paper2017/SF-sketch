#include "sketch_config.h"

uint32_t hash(uint32_t row, const unsigned char * str, size_t len) {
    return bobhash(row, str, len);
}
#ifdef FC_V1
uint32_t hash_l(uint32_t row, const unsigned char * str, size_t len) {
    return bobhash(row + MAX_ROW, str, len);
}
#endif
