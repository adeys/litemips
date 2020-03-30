#ifndef LMIPS_MIPS
#define LMIPS_MIPS

#include "common.h"
#include "lmips_registers.h"

#define STACK_SIZE 1024

struct lm {
    uint8_t* program;
    int32_t regs[REG_COUNT];
    uint32_t ip;
    int32_t hi, lo;
    int32_t stack[STACK_SIZE];
    bool stop;
};

typedef enum {
    EXEC_SUCCESS,
    EXEC_FAILURE,
    EXEC_EXCP_INT_OVERFLOW
} ExecutionResult ;

typedef struct lm LMips;

void initSimulator(LMips* mips, uint8_t* program);
void freeSimulator(LMips* mips);
ExecutionResult runSimulator(LMips* mips);
ExecutionResult execInstruction(LMips* mips);

uint32_t zero_extend(uint16_t x, int bit_count);
int32_t sign_extend(int16_t x, int bit_count);
void handleException(ExecutionResult);

#endif // LMIPS_MIPS
