#ifndef LMIPS_MIPS
#define LMIPS_MIPS

#include "common.h"
#include "memory.h"
#include "lmips_registers.h"

struct lm {
    uint8_t* program;
    uint32_t regs[REG_COUNT];
    uint32_t ip;
    int32_t hi, lo;
    Memory* memory;
    bool stop;
};

typedef enum {
    EXEC_SUCCESS,
    EXEC_FAILURE,
    EXEC_ERR_INT_OVERFLOW,
    EXEC_ERR_MEMORY_ADDR
} ExecutionResult ;

typedef struct lm LMips;

void initTestSimulator(LMips* mips, uint8_t* program);
void initSimulator(LMips* mips, Memory* memory);
void freeSimulator(LMips* mips);
ExecutionResult runSimulator(LMips* mips);
ExecutionResult execInstruction(LMips* mips);

void handleException(ExecutionResult);

#endif // LMIPS_MIPS
