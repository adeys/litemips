#ifndef LMIPS_OPCODES
#define LMIPS_OPCODES

enum OpCodes {
    OP_ALU,
    OP_BEQ = 0x04,
    OP_BNE,
    OP_BLEZ,
    OP_BGTZ,
    OP_ADDI = 0x08,
    OP_ADDIU,
    OP_SLTI,
    OP_SLTIU,
    OP_ANDI,
    OP_ORI,
    OP_XORI,
    OP_LB = 0x20,
    OP_LH,
    OP_LW = 0x23,
    OP_LBU,
    OP_LHU,
    OP_SB = 0x28,
    OP_SH
};

enum AluCodes {
    OP_SLL,
    OP_SRL = 0x02,
    OP_SRA,
    OP_SLLV,
    OP_SRLV = 0x06,
    OP_SRAV,
    OP_JR,
    OP_JALR,
    OP_SYSCALL = 0x0D,
    OP_MFHI = 0x10,
    OP_MTHI,
    OP_MFLO,
    OP_MTLO,
    OP_MULT = 0x18,
    OP_MULTU,
    OP_DIV,
    OP_DIVU,
    OP_ADD = 0x20,
    OP_ADDU,
    OP_SUB,
    OP_SUBU,
    OP_AND,
    OP_OR,
    OP_XOR,
    OP_NOR,
    OP_SLT = 0x2A,
    OP_SLTU
};

enum SysCallCodes {
    SYS_PRINT_INT = 0x01,
    SYS_PRINT_STRING = 0x04,
    SYS_READ_INT,
    SYS_READ_STRING = 0x08,
    SYS_EXIT = 0x10
}

#endif // LMIPS_OPCODES
