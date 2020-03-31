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

bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isAlphaNum(char c) {
    return c == '_' || isAlpha(c) || isDigit(c);
}

bool isHexDigit(char c) {
    return (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || isDigit(c);
}
bool isOctDigit(char c) {
    return c >= '0' && c <= '7';
}