#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "executable.h"
#include "lmips.h"

#define HEADER_SIZE (120 / 8)

uint32_t read_word(FILE* file) {
    uint32_t word;
    fread(&word, sizeof(uint32_t), 1, file);

    return ((word & 0x000000FF) << 24) |
        ((word & 0x0000FF00) << 8)  |
        ((word & 0x00FF0000) >> 8) |
        ((word & 0xFF000000) >> 24);
}

uint16_t read_half(FILE* file) {
    uint16_t half;
    fread(&half, sizeof(uint16_t), 1, file);

    return (half >> 8) | (half << 8);
}

uint8_t read_byte(FILE* file) {
    uint8_t byte;
    fread(&byte, sizeof(uint8_t), 1, file);

    return byte;
}

FileHeader getHeader(FILE* file, const char* fileName) {
    FileHeader header;
    char format[4] = {0x10, 'L', 'E', 'F'};

    fread(header.magic, sizeof(char), 4, file);
    if (memcmp(header.magic, format, 4) != 0) {
        printf("File '%s' is not a valid executable file.\n", fileName);
        fclose(file);
        exit(1);
    }

    header.major = read_byte(file);
    header.minor = read_byte(file);
    header.entry = read_word(file);
    header.shAddress = read_word(file);
    header.shCount = read_byte(file);

    header.size = HEADER_SIZE;

    return header;
}

int main(int argc, char const *argv[]) {
    if (argc < 2 || argc > 2) {
        printf("Usage : lms [file]\n");
        exit(1);
    }

    FILE* source;
    source = fopen(argv[1], "rw+");
    if (source == NULL) {
        printf("Unable to open file '%s'.\n", argv[1]);
        exit(1);
    }

    // Get file header
    FileHeader header = getHeader(source, argv[1]);

    // Get section header table
    SectionHeader sections[header.shCount];
    fseek(source, header.shAddress, SEEK_SET);
    for (int i = 0; i < header.shCount; ++i) {
        SectionHeader section;
        section.name = read_half(source);
        section.type = read_byte(source);
        section.address = read_word(source);
        section.size = read_word(source);

        sections[i] = section;
    }

    Memory memory = {};
    initMemory(&memory);
    uint32_t programOffset = PROGRAM_OFFSET;
    uint32_t dataOffset = DATA_OFFSET;

    // Start sections reading
    for (int i = 0; i < header.shCount; ++i) {
        SectionHeader section = sections[i];
        fseek(source, section.address, SEEK_SET);
        switch (section.type) {
            case SHT_EXEC: {
                for (int j = 0; j < section.size * 0.25; ++j) {
                    uint32_t instr = read_word(source);
                    mem_write(&memory, programOffset, instr);
                    programOffset += 4;
                }
                break;
            }
            case SHT_ALLOC: {
                for (int j = 0; j < section.size; ++j) {
                    uint8_t buffer = read_byte(source);
                    mem_write_byte(&memory, dataOffset++, buffer);
                }
                break;
            }
            case SHT_STRTAB: {
                read_byte(source);
                for (int j = 0; j < section.size - 2; ++j) {
                    uint8_t buffer = read_byte(source);
                    mem_write_byte(&memory, dataOffset++, buffer);
                }
                read_byte(source);
                break;
            }
            case SHT_NULL:
                break;
        }
    }

    fclose(source);

    LMips mips;
    initSimulator(&mips, &memory);
    mips.ip = header.entry - header.size;

    runSimulator(&mips);

    freeSimulator(&mips);
    freeMemory(&memory);
    return 0;
}
