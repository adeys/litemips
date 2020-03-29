#include <stdlib.h>

#include "lmips.h"

void resetSimulator(LMips* mips) {
    mips->program = NULL;
    mips->ip = 0;
    mips->hi = 0;
    mips->lo = 0;

    // Init all registers to 0
    for (size_t i = 0; i < REG_COUNT; i++) {
        mips->regs[i] = 0;
    }
}

void initSimulator(LMips* mips) {
    resetSimulator(mips);
}

void freeSimulator(LMips* mips) {
    resetSimulator(mips);
}