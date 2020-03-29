#ifndef LMIPS_MIPS
#define LMIPS_MIPS

#include "common.h"
#include "lmips_registers.h"

#define STACK_SIZE 1024

struct lm {
    uint8_t* program;
    uint32_t regs[REG_COUNT];
    uint32_t ip, hi, lo;
    int32_t stack[STACK_SIZE];
};

typedef struct lm LMips;

void initSimulator(LMips* mips);
void freeSimulator(LMips* mips);

#endif // LMIPS_MIPS
