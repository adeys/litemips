#include "memory.h"

void resetMemory(Memory* memory) {
    for (int i = 0; i < MEMORY_SIZE; ++i) {
        memory->store[i] = 0;
    }
}

void initMemory(Memory* memory) {
    resetMemory(memory);
}

void freeMemory(Memory* memory) {
    resetMemory(memory);
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