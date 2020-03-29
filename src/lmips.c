#include <stdio.h>

#include "lmips.h"
#include "lmips_opcodes.h"

bool checkOverflow(int x) {
    return x > INT32_MAX || x < INT32_MIN;
}

void resetSimulator(LMips* mips) {
    mips->ip = 0;
    mips->hi = 0;
    mips->lo = 0;
    mips->stop = false;
    mips->program = NULL;

    // Init all registers to 0
    for (size_t i = 0; i < REG_COUNT; i++) {
        mips->regs[i] = 0;
    }
}

void initSimulator(LMips* mips, uint8_t* program) {
    resetSimulator(mips);
    mips->regs[$sp] = STACK_SIZE;
    mips->program = program;
}

void freeSimulator(LMips* mips) {
    resetSimulator(mips);
}

ExecutionResult runSimulator(LMips* mips) {
    if (mips->program == NULL) {
        fprintf(stderr, "Invalid program provided.\n");
        return EXEC_FAILURE;
    }

    ExecutionResult result = EXEC_FAILURE;
    while (!mips->stop) {
        result = execInstruction(mips);
        if (result != EXEC_SUCCESS) {
            mips->stop = true;
            handleException(result);
        }
    }

    return result;
}

ExecutionResult execInstruction(LMips* mips) {

#define GET_INSTR() \
    ((mips->program[mips->ip++] << 0x18) | \
        (mips->program[mips->ip++] << 0x10) | \
        (mips->program[mips->ip++] << 0x8) | \
        (mips->program[mips->ip++]) \
    )
#define GET_OP(instr) (instr >> 0x1A)
#define GET_RS(instr) ((instr >> 0x15) & 0x1F)
#define GET_RT(instr) ((instr >> 0x10) & 0x1F)
#define GET_RD(instr) ((instr >> 0x0B) & 0x1F)
#define GET_SA(instr) ((instr >> 0x06) & 0x1F)
#define GET_FUNC(instr) (instr & 0x3F)
#define BIN_OP(op) \
    do { \
        int res = mips->regs[GET_RS(instr)] op mips->regs[GET_RT(instr)]; \
        if (checkOverflow(res)) { \
            return EXEC_EXCP_INT_OVERFLOW; \
        } \
\
        uint8_t rd = GET_RD(instr); \
        mips->regs[rd] = res;\
    } while(false)
#define BINU_OP(op) (mips->regs[GET_RD(instr)] = mips->regs[GET_RS(instr)] op mips->regs[GET_RT(instr)])

    uint32_t instr = GET_INSTR();
    uint8_t op = GET_OP(instr);

    switch (op) {
        case OP_SPECIAL: {
            uint8_t func = GET_FUNC(instr);
            switch (func) {
                case SPE_ADD: {
                    BIN_OP(+);
                    break;
                }
                case SPE_ADDU: {
                    BINU_OP(+);
                    break;
                }
                case SPE_SUB: {
                    BIN_OP(-);
                    break;
                }
                case SPE_SUBU: {
                    BINU_OP(-);
                    break;
                }
                case SPE_MULT:
                case SPE_MULTU: {
                    int64_t result = mips->regs[GET_RS(instr)] * mips->regs[GET_RT(instr)];
                    mips->hi = result >> 0x20;
                    mips->lo = (int32_t)result;
                    break;
                }
                case SPE_DIV:
                case SPE_DIVU: {
                    int32_t rs = mips->regs[GET_RS(instr)];
                    int32_t rt = mips->regs[GET_RT(instr)];

                    mips->hi = rs % rt;
                    mips->lo = rs / rt;
                    break;
                }
                case SPE_SYSCALL: {
                    mips->stop = true;
                    break;
                }
                default:
                    fprintf(stderr, "Unknown special instruction %d\n", func);
                    return EXEC_FAILURE;
            }

            return EXEC_SUCCESS;
        }
        default:
            fprintf(stderr, "Unknown instruction %d\n", op);
            return EXEC_FAILURE;
    }
}


uint32_t sign_extend(uint32_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }

    return x;
}

void handleException(ExecutionResult exc) {
    if (exc == EXEC_EXCP_INT_OVERFLOW) {
        fprintf(stderr, "Integer overflow exception.");
    }
}