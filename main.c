#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "executable.h"
#include "lmips.h"

#define HEADER_SIZE 120

FileHeader getHeader(FILE* file, const char* fileName) {
    FileHeader header;
    char format[4] = {0x10, 'L', 'E', 'F'};

    fread(header.magic, sizeof(char), 4, file);
    if (memcmp(header.magic, format, 4) != 0) {
        printf("File '%s' is not a valid executable file.\n", fileName);
        fclose(file);
        exit(1);
    }

    fread(&header.major, sizeof(uint8_t), 1, file);
    fread(&header.minor, sizeof(uint8_t), 1, file);
    fread(&header.entry, sizeof(uint32_t), 1, file);
    fread(&header.shAddress, sizeof(uint32_t), 1, file);
    fread(&header.shCount, sizeof(uint8_t), 1, file);

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
        fread(&section.name, sizeof(uint16_t), 1, source);
        fread(&section.type, sizeof(uint8_t), 1, source);
        fread(&section.address, sizeof(uint32_t), 1, source);
        fread(&section.size, sizeof(uint32_t), 1, source);

        sections[i] = section;
    }

    Memory memory;
    initMemory(&memory);
    uint32_t programOffset = PROGRAM_OFFSET;
    uint32_t dataOffset = DATA_OFFSET;
    // Start sections reading
    for (int i = 0; i < header.shCount; ++i) {
        SectionHeader section = sections[i];
        fseek(source, section.address, SEEK_SET);
        switch (section.type) {
            case SHT_EXEC: {
                for (int j = 0; j < section.size / sizeof(uint32_t); ++j) {
                    uint32_t instr;
                    fread(&instr, sizeof(uint32_t), 1, source);
                    mem_write(&memory, programOffset, instr);
                    programOffset += 4;
                }
                break;
            }
            case SHT_ALLOC: {
                for (int j = 0; j < section.size / sizeof(uint32_t); ++j) {
                    uint32_t buffer;
                    fread(&buffer, sizeof(uint32_t), 1, source);
                    mem_write(&memory, programOffset, buffer);
                    dataOffset += 4;
                }
                break;
            }
            case SHT_STRTAB:
            case SHT_NULL:
                break;
        }
    }

    fclose(source);

    LMips mips;
    initSimulator(&mips, &memory);

    runSimulator(&mips);

    freeSimulator(&mips);
    freeMemory(&memory);
    return 0;
}
