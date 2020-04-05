#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

void initMemory(Memory* memory) {
    memory->store = realloc(NULL, MEMORY_SIZE * sizeof(uint8_t));
}

void freeMemory(Memory* memory) {
    memory->store = realloc(memory->store, 0);
}

int32_t mem_read(Memory* memory, uint32_t address) {
    return memory->store[address + 3] |
           (memory->store[address + 2] << 0x08) |
           (memory->store[address + 1] << 0x10) |
           (memory->store[address] << 0x18);
}

uint8_t mem_read_byte(Memory* memory, uint32_t address) {
    return memory->store[address];
}

uint16_t mem_read_half(Memory* memory, uint32_t address) {
    return memory->store[address + 1] | (memory->store[address] << 0x08);
}

void mem_write(Memory* memory, uint32_t address, uint32_t value) {
    memory->store[address + 3] = value;
    memory->store[address + 2] = (uint8_t)(value >> 0x08);
    memory->store[address + 1] = (uint8_t)(value >> 0x10);
    memory->store[address] = (uint8_t)(value >> 0x18);
}

void mem_write_byte(Memory* memory, uint32_t address, uint8_t value) {
    memory->store[address] = value;
}

void mem_write_half(Memory* memory, uint32_t address, uint16_t value) {
    memory->store[address + 1] = value;
    memory->store[address] = (uint8_t)(value >> 0x08);
}