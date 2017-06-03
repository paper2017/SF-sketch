#include "sketch_config.h"

uint32_t hash(uint32_t row, const unsigned char * str, size_t len)
{
    return bobhash(row, str, len);
}

#if (SF1 + SF2 == 1)
uint32_t hash2(uint32_t row, const unsigned char * str, size_t len)
{
    return bobhash(row + MAX_ROW, str, len);
}
#endif
