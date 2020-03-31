#ifndef LMIPS_COMMON
#define LMIPS_COMMON

#include <stdbool.h>
#include <stdint.h>

int32_t zero_extend(uint16_t x, int bit_count);
int32_t sign_extend(int16_t x, int bit_count);

bool isAlpha(char c);
bool isDigit(char c);
bool isAlphaNum(char c);
bool isHexDigit(char c);
bool isOctDigit(char c);

#endif // LMIPS_COMMON
