#ifndef LMIPS_INSTRUCTIONS
#define LMIPS_INSTRUCTIONS


typedef enum {
    // Arithmetic and Logical instructions
    I_ABS,
    I_ADD, I_ADDU, I_ADDI, I_ADDIU,
    I_AND, I_ANDI,
    I_DIV, I_DIVU,
    I_MULT, I_MULTU,
    I_NEG, I_NEGU,
    I_NOR,
    I_NOT,
    I_OR,
    I_REM, I_REMU,
    I_SLL,
    I_SLLV,
    I_SRA,
    I_SRAV,
    I_SRL,
    I_SRLV,
    I_SUB, I_SUBU,
    I_XOR, I_XORI,
    I_LI,
    I_SLT, I_SLTU, I_SLTI, I_SLTIU,
    I_SEQ,
    I_SGE, I_SGEU,
    I_SGT, I_SGTU,
    I_SLE, I_SLEU,
    I_SNE,

    // Branch instructions
    I_B,
    I_BEQ,
    I_BGEZ,
    I_BGTZ,
    I_BLEZ,
    I_BLTZ,
    I_BNE,
    I_BEQZ,
    I_BGE, I_BGEU,
    I_BGT, I_BGTU,
    I_BLE, I_BLEU,
    I_BLT, I_BLTU,
    I_BNEZ,

    // Jump instructions
    I_J,
    I_JAL,
    I_JALR,
    I_JR,

    // Load instructions
    I_LA,
    I_LB, I_LBU,
    I_LH, I_LHU,
    I_LW,

    // Store instructions
    I_SB,
    I_SH,

    // Data movements
    I_MOVE,
    I_MFHI,
    I_MFLO,
    I_MTHI,
    I_MTLO,

    I_SYSCALL
} InstructionType;

typedef enum {
    D_ASCII,
    D_ASCIIZ,
    D_BYTE,
    D_DATA,
    D_HALF,
    D_SPACE,
    D_TEXT,
    D_WORD
} DirectiveType;

#endif //LMIPS_INSTRUCTIONS
