#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lmips.h"
#include "lmips_opcodes.h"

void resetSimulator(LMips* mips) {
    mips->ip = 0;
    mips->hi = 0;
    mips->lo = 0;
    mips->stop = false;
    mips->program = NULL;
    mips->memory = NULL;

    // Init all registers to 0
    for (size_t i = 0; i < REG_COUNT; i++) {
        mips->regs[i] = 0;
    }
}

void initTestSimulator(LMips* mips, uint8_t* program) {
    resetSimulator(mips);
    mips->regs[$sp] = STACK_ADDRESS;
    mips->regs[$gp] = HEAP_ADDRESS;
    mips->program = program;
}

void initSimulator(LMips* mips, Memory* memory) {
    resetSimulator(mips);
    mips->regs[$sp] = STACK_ADDRESS;
    mips->regs[$gp] = HEAP_ADDRESS;
    mips->program = &memory->store[PROGRAM_ADDRESS];

    mips->memory = memory;
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
            handleException(result, mips);
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
#define CHECK_MEM_ADDR(offset, align, address) \
    if ((offset % align != 0) || address >= MEMORY_SIZE || address < DATA_ADDRESS) return EXEC_ERR_MEMORY_ADDR
#define COMP_OP(op) \
    if ((int32_t)(mips->regs[GET_RS(instr)]) op 0) { \
        uint32_t offset = GET_IMMED(instr) << 2; \
        mips->ip += (offset - 4); \
    }

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
                case SPE_SRL: {
                    mips->regs[GET_RD(instr)] = mips->regs[GET_RT(instr)] >> GET_SA(instr);
                    break;
                }
                case SPE_SRA: {
                    mips->regs[GET_RD(instr)] = (int32_t)mips->regs[GET_RT(instr)] >> GET_SA(instr);
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
                        case SYS_PRINT_INT: {
                            printf("%d", mips->regs[$a0]);
                            break;
                        }
                        case SYS_PRINT_STRING: {
                            const char* string = (const char*)&mips->memory->store[mips->regs[$a0]];
                            printf("%s", string);
                            fflush(stdout);
                            break;
                        }
                        case SYS_READ_INT: {
                            char buffer[12];
                            fgets(buffer, 11, stdin);
                            buffer[strlen(buffer)] = '\0';
                            mips->regs[$v0] = strtoul(buffer, NULL, 0);
                            break;
                        }
                        case SYS_READ_STRING: {
                            uint32_t address = mips->regs[$a0];
                            fgets((char*)&mips->memory->store[address], mips->regs[$a1], stdin);
                            mips->memory->store[address + strlen((char*)&mips->memory->store[address]) - 1] = '\0';
                            break;
                        }
                        case SYS_SBRK: {
                            mips->regs[$gp] += mips->regs[$a0];
                            mips->regs[$v0] = mips->regs[$gp];
                            break;
                        }
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
        case OP_SRI: {
            uint8_t rt = GET_RT(instr);
            switch (rt) {
                case SR_BLTZ: {
                    COMP_OP(<)
                    break;
                }
                case SR_BGEZ: {
                    COMP_OP(>=)
                    break;
                }
                default: {
                    fprintf(stderr, "Unknown regimm instruction %d.", rt);
                    return EXEC_FAILURE;
                }
            }
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
                int32_t offset = GET_IMMED(instr) << 2;
                mips->ip += (offset - 4);
            }
            break;
        }
        case OP_BNE: {
            if (mips->regs[GET_RS(instr)] != mips->regs[GET_RT(instr)]) {
                uint32_t offset = GET_IMMED(instr) << 2;
                mips->ip += (offset - 4);
            }
            break;
        }
        case OP_BLEZ: {
            COMP_OP(<=)
            break;
        }
        case OP_BGTZ: {
            COMP_OP(>)
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

            mips->regs[GET_RT(instr)] = (int32_t)(mips->regs[GET_RS(instr)]) + immed;
            break;
        }
        case OP_SLTI: {
            int32_t immed = sign_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = (int32_t)(mips->regs[GET_RS(instr)]) < immed;
            break;
        }
        case OP_SLTIU: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = (int32_t)(mips->regs[GET_RS(instr)]) < immed;
            break;
        }
        case OP_ANDI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] & immed;
            break;
        }
        case OP_ORI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] | immed;
            break;
        }
        case OP_XORI: {
            uint32_t immed = zero_extend(GET_IMMED(instr), 16);

            mips->regs[GET_RT(instr)] = mips->regs[GET_RS(instr)] ^ immed;
            break;
        }
        case OP_LUI: {
            uint32_t immed = (GET_IMMED(instr) << 16) | 0x00;
            mips->regs[GET_RT(instr)] = immed;
            break;
        }
        case OP_LB: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 1, address);
            int8_t byte = mem_read_byte(mips->memory, address);

            mips->regs[GET_RT(instr)] = sign_extend(byte, 16);
            break;
        }
        case OP_LH: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 2, address);
            int16_t half = mem_read_half(mips->memory, address);

            mips->regs[GET_RT(instr)] = sign_extend(half, 16);
            break;
        }
        case OP_LW: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 4, address);

            mips->regs[GET_RT(instr)] = (int32_t)mem_read(mips->memory, address);
            break;
        }
        case OP_LBU: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 1, address);
            uint8_t byte = mem_read_byte(mips->memory, address);

            mips->regs[GET_RT(instr)] = zero_extend(byte, 16);
            break;
        }
        case OP_LHU: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 2, address);

            mips->regs[GET_RT(instr)] = mem_read_half(mips->memory, address);
            break;
        }
        case OP_SB: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 1, address);

            mem_write_byte(mips->memory, address, (uint8_t)mips->regs[GET_RT(instr)]);

            break;
        }
        case OP_SH: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 1, address);

            mem_write_half(mips->memory, address, mips->regs[GET_RT(instr)]);

            break;
        }
        case OP_SW: {
            int16_t offset = GET_IMMED(instr);
            uint32_t address = mips->regs[GET_RS(instr)] + offset;
            CHECK_MEM_ADDR(offset, 1, address);

            mem_write(mips->memory, address, mips->regs[GET_RT(instr)]);

            break;
        }
        default:
            fprintf(stderr, "Unknown instruction %d\n", op);
            return EXEC_FAILURE;
    }

    return EXEC_SUCCESS;
}


void handleException(ExecutionResult exc, LMips* mips) {
    if (exc == EXEC_ERR_INT_OVERFLOW) {
        fprintf(stderr, "[%#08x] Integer overflow exception.\n", PROGRAM_ADDRESS + mips->ip);
    } else if (exc == EXEC_ERR_MEMORY_ADDR) {
        fprintf(stderr, "[%#08x] Invalid memory address.\n", PROGRAM_ADDRESS + mips->ip);
    }
}
