#ifndef LMIPS_MEMORY
#define LMIPS_MEMORY

#include "common.h"

#define MEMORY_SIZE ((UINT16_MAX + 1) * 64) // 4MB
#define PROGRAM_OFFSET 0x02000
#define DATA_OFFSET 0x07FFFF
#define STACK_OFFSET 0x3FFFF0

typedef struct {
    uint8_t* store;
} Memory;

void initMemory(Memory* memory);
void freeMemory(Memory* memory);

int32_t mem_read(Memory* memory, uint32_t address);
uint8_t mem_read_byte(Memory* memory, uint32_t address);
uint16_t mem_read_half(Memory* memory, uint32_t address);

void mem_write(Memory* memory, uint32_t address, uint32_t value);
void mem_write_byte(Memory* memory, uint32_t address, uint8_t value);
void mem_write_half(Memory* memory, uint32_t address, uint16_t value);

#endif //LMIPS_MEMORY
