#include <stdio.h>

#include "lmips.h"
#include "lmips_opcodes.h"

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
    mips->regs[$sp] = STACK_SIZE - 1;
    mips->regs[$gp] = MEMORY_SIZE - 1;
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
#define GET_IMMED(instr) (instr & 0xFFFF)
#define GET_JT(instr) (instr & 0x3FFFFFF)
#define CHECK_OVERFLOW(x, y, op) \
    do { \
        int64_t res = (int64_t)x op y;\
        if (res > INT32_MAX || res < INT32_MIN) { \
            return EXEC_ERR_INT_OVERFLOW; \
        } \
    } while(false)
#define BIN_OP(op) \
    do { \
        int32_t rs = mips->regs[GET_RS(instr)]; \
        int32_t rt = mips->regs[GET_RT(instr)]; \
        CHECK_OVERFLOW(rs, rt, op); \
\
        uint8_t rd = GET_RD(instr); \
        mips->regs[rd] = rs op rt;\
    } while(false)
#define BINU_OP(op) (mips->regs[GET_RD(instr)] = mips->regs[GET_RS(instr)] op mips->regs[GET_RT(instr)])
#define CHECK_MEM_ADDR(offset, address) \
    if ((offset % 4 != 0) || address >= MEMORY_SIZE) return EXEC_ERR_MEMORY_ADDR

    uint32_t instr = GET_INSTR();
    uint8_t op = GET_OP(instr);

    switch (op) {
        case OP_SPECIAL: {
            uint8_t func = GET_FUNC(instr);
            switch (func) {
                case SPE_SLL: {
                    mips->regs[GET_RD(instr)] = mips->regs[GET_RT(instr)] << GET_SA(instr);
                    break;
                }
                case SPE_SRL:
                case SPE_SRA: {
                    mips->regs[GET_RD(instr)] = mips->regs[GET_RT(instr)] >> GET_SA(instr);
                    break;
                }
                case SPE_SLLV: {
                    uint8_t amount = mips->regs[GET_RS(instr)] & 0x1F;
                    mips->regs[GET_RD(instr)] = mips->regs[GET_RT(instr)] << amount;
                    break;
                }
                case SPE_SRLV:
                case SPE_SRAV: {
                    uint8_t amount = mips->regs[GET_RS(instr)] & 0x1F;
                    mips->regs[GET_RD(instr)] = mips->regs[GET_RT(instr)] >> amount;
                    break;
                }
                case SPE_JR: {
                    uint32_t rs = mips->regs[GET_RS(instr)];
                    mips->ip = rs;
                    break;
                }
                case SPE_JALR: {
                    uint8_t rd = GET_RD(instr);
                    mips->regs[rd <= 0 ? $ra : rd] = mips->ip;
                    mips->ip = mips->regs[GET_RS(instr)];
                    break;
                }
                case SPE_SYSCALL: {
                    switch (mips->regs[$v0]) {
                        case SYS_EXIT: {
                            mips->stop = true;
                            break;
                        }
                        default: {
                            fprintf(stderr, "Unknown syscall instruction %d\n", mips->regs[$v0]);
                            return EXEC_FAILURE;
                        }
                    }
                    break;
                }
                case SPE_MFHI: {
                    mips->regs[GET_RD(instr)] = mips->hi;
                    break;
                }
                case SPE_MTHI: {
                    mips->hi = mips->regs[GET_RS(instr)];
                    break;
                }
                case SPE_MFLO: {
                    mips->regs[GET_RD(instr)] = mips->lo;
                    break;
                }
                case SPE_MTLO: {
                    mips->hi = mips->regs[GET_RS(instr)];
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

                    if (rt != 0) {
                        mips->hi = rs % rt;
                        mips->lo = rs / rt;
                    }

                    break;
                }
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
                case SPE_AND: {
                    BINU_OP(&);
                    break;
                }
                case SPE_OR: {
                    BINU_OP(|);
                    break;
                }
                case SPE_XOR: {
                    BINU_OP(^);
                    break;
                }
                case SPE_NOR: {
                    mips->regs[GET_RD(instr)] = ~(mips->regs[GET_RS(instr)] | mips->regs[GET_RT(instr)]);
                    break;
                }
                case SPE_SLT:
                case SPE_SLTU: {
                    mips->regs[GET_RD(instr)] = (mips->regs[GET_RS(instr)] < mips->regs[GET_RT(instr)]);
                    break;
                }
                default:
                    fprintf(stderr, "Unknown special instruction %d\n", func);
                    return EXEC_FAILURE;
            }

            break;
        }
        case OP_J: {
            int32_t jt = GET_JT(instr);
            mips->ip = jt << 2;
            break;
        }
        case OP_JAL: {
            int32_t jt = GET_JT(instr);
            mips->regs[$ra] = mips->ip;
            mips->ip = jt << 2;
            break;
        }
        case OP_BEQ: {
            if (mips->regs[GET_RS(instr)] == mips->regs[GET_RT(instr)]) {
                uint32_t offset = GET_IMMED(instr) << 2;
                mips->ip += offset;
            }
            break;
        }
        case OP_BNE: {
            if (mips->regs[GET_RS(instr)] != mips->regs[GET_RT(instr)]) {
                uint32_t offset = GET_IMMED(instr) << 2;
                mips->ip += offset;
            }
            break;
        }
        case OP_BLEZ: {
            if (mips->regs[GET_RS(instr)] <= 0) {
                uint32_t offset = GET_IMMED(instr) << 2;
                mips->ip += offset;
            }
            break;
        }
        case OP_BGTZ: {
            if (mips->regs[GET_RS(instr)] >= 0) {
                uint32_t offset = GET_IMMED(instr) << 2;
                mips->ip += offset;
            }
            break;
        }
        case OP_ADDI: {
            int32_t immed = sign_extend(GET_IMMED(instr), 16);
            int32_t rs = mips->regs[GET_RS(instr)];
            CHECK_OVERFLOW(rs, immed, +);

            mips->regs[GET_RT(instr)] = rs + immed;
            break;
        }
        case OP_ADDIU: {
            int32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] + immed;
            break;
        }
        case OP_SLTI: {
            int32_t immed = sign_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] < immed;
            break;
        }
        case OP_SLTIU: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] < immed;
            break;
        }
        case OP_ANDI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = GET_RS(instr) & immed;
            break;
        }
        case OP_ORI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = GET_RS(instr) | immed;
            break;
        }
        case OP_XORI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = GET_RS(instr) ^ immed;
            break;
        }
        case OP_LB: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);
            int16_t byte = mem_read_byte(mips, address, true);

            mips->regs[GET_RT(instr)] = sign_extend(byte, 16);
            break;
        }
        case OP_LH: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);

            mips->regs[GET_RT(instr)] = mem_read_half(mips, address, true);
            break;
        }
        case OP_LW: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);

            mips->regs[GET_RT(instr)] = mem_read(mips, address);
            break;
        }
        case OP_LBU: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);
            uint16_t byte = mem_read_byte(mips, address, false);

            mips->regs[GET_RT(instr)] = sign_extend(byte, 16);
            break;
        }
        case OP_LHU: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);

            mips->regs[GET_RT(instr)] = mem_read_half(mips, address, false);
            break;
        }
        case OP_SB: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);

            mem_write_byte(mips, address, mips->regs[GET_RT(instr)]);

            break;
        }
        case OP_SH: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + (offset / 4);
            CHECK_MEM_ADDR(offset, address);

            mem_write_half(mips, address, mips->regs[GET_RT(instr)]);

            break;
        }
        default:
            fprintf(stderr, "Unknown instruction %d\n", op);
            return EXEC_FAILURE;
    }

    return EXEC_SUCCESS;
}

int32_t zero_extend(uint16_t x, int bit_count) {
    return x | (0x00 << bit_count);
}

int32_t sign_extend(int16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }

    return x;
}

int16_t sign_extend_byte(int8_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0x1F << bit_count);
    }

    return x;
}

int16_t zero_extend_byte(uint8_t x, int bit_count) {
    return x | (0x00 << bit_count);
}

void handleException(ExecutionResult exc) {
    if (exc == EXEC_ERR_INT_OVERFLOW) {
        fprintf(stderr, "Integer overflow exception.\n");
    } else if (exc == EXEC_ERR_MEMORY_ADDR) {
        fprintf(stderr, "Invalid memory address.\n");
    }
}

int32_t mem_read(LMips* mips, uint32_t address) {
    return mips->memory[address];
}

int16_t mem_read_byte(LMips* mips, uint32_t address, bool sign) {
    int16_t byte = mips->memory[address] & 0x1F;
    return sign ? sign_extend_byte(byte, 8) : zero_extend_byte(byte, 8);
}

int32_t mem_read_half(LMips* mips, uint32_t address, bool sign) {
    int32_t half = mips->memory[address] & 0xFFFF;
    return sign ? sign_extend(half, 16) : zero_extend(half, 16);
}

void mem_write(LMips* mips, uint32_t address, int32_t value) {
    mips->memory[address] = value;
}

void mem_write_byte(LMips* mips, uint32_t address, int32_t value) {
    mips->memory[address] = value;
}

void mem_write_half(LMips* mips, uint32_t address, int32_t value) {
    mips->memory[address] = value;
}