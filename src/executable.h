#ifndef LMIPS_EXECUTABLE_H
#define LMIPS_EXECUTABLE_H

#include "common.h"

typedef struct {
    char magic[4];
    uint8_t major;
    uint8_t minor;
    uint32_t entry;
    uint32_t shAddress;
    uint8_t shCount;
    uint8_t size;
} FileHeader;

typedef enum {
    SHT_NULL,
    SHT_EXEC,
    SHT_STRTAB,
    SHT_ALLOC
} SectionType;

typedef struct {
    uint16_t name;
    SectionType type;
    uint32_t address;
    uint32_t size;
} SectionHeader;

#endif //LMIPS_EXECUTABLE_H
