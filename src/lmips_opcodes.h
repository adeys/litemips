#ifndef LMIPS_OPCODES
#define LMIPS_OPCODES

enum OpCodes {
    OP_SPECIAL,
    OP_J = 0x02,
    OP_JAL,
    OP_BEQ,
    OP_BNE,
    OP_BLEZ,
    OP_BGTZ,
    OP_ADDI,
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

enum SpecialCodes {
    SPE_SLL,
    SPE_SRL = 0x02,
    SPE_SRA,
    SPE_SLLV,
    SPE_SRLV = 0x06,
    SPE_SRAV,
    SPE_JR,
    SPE_JALR,
    SPE_SYSCALL = 0x0D,
    SPE_MFHI = 0x10,
    SPE_MTHI,
    SPE_MFLO,
    SPE_MTLO,
    SPE_MULT = 0x18,
    SPE_MULTU,
    SPE_DIV,
    SPE_DIVU,
    SPE_ADD = 0x20,
    SPE_ADDU,
    SPE_SUB,
    SPE_SUBU,
    SPE_AND,
    SPE_OR,
    SPE_XOR,
    SPE_NOR,
    SPE_SLT = 0x2A,
    SPE_SLTU
};

enum SysCallCodes {
    SYS_PRINT_INT = 0x01,
    SYS_PRINT_STRING = 0x04,
    SYS_READ_INT,
    SYS_READ_STRING = 0x08,
    SYS_EXIT = 0x10
};

#endif // LMIPS_OPCODES
