#include "common.h"

int32_t zero_extend(uint16_t x, int bit_count) {
    return x | (0x00 << bit_count);
}

int32_t sign_extend(int16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }

    return x;
}
