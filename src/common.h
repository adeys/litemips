#ifndef LMIPS_COMMON
#define LMIPS_COMMON

#include <stdbool.h>
#include <stdint.h>

int32_t zero_extend(uint16_t x, int bit_count);
int32_t sign_extend(int16_t x, int bit_count);

#endif // LMIPS_COMMON
