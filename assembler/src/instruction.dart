import 'token.dart';

enum InstructionType {
  R_TYPE,
  RL_TYPE, // R_type based pseudo-instructions
  J_TYPE,
  I_TYPE
}

class Instruction {
  String name;
  int opCount;
  Token rd;
  Token rs;
  Token rt;
  Token shmt;
  Token immed;
  List<Token> operands;
  InstructionType type;

  Instruction(this.name, this.opCount, this.type);
}

const Map<String, int> OpCodes = {
  "rsi": 0x01,
  "j": 0x02,
  "jal": 0x03,
  "beq": 0x04,
  "bne": 0x05,
  "blez": 0x06,
  "bgtz": 0x07,
  "bltz": 0x00,
  "bgez": 0x01,
  "addi": 0x08,
  "addiu": 0x09,
  "slti": 0x0A,
  "sltiu": 0x0B,
  "andi": 0x0C,
  "ori": 0x0D,
  "xori": 0x0E,
  "lui": 0x0F,
  "lb": 0x20,
  "lh": 0x21,
  "lw": 0x23,
  "lbu": 0x24,
  "lhu": 0x25,
  "sb": 0x28,
  "sh": 0x29,
  "sw": 0x2A,

  // ALU functions
  "sll": 0x00,
  "srl": 0x02,
  "sra": 0x03,
  "sllv": 0x04,
  "srlv": 0x06,
  "srav": 0x07,
  "jr": 0x08,
  "jarl": 0x09,
  "syscall": 0x0C,
  "mfhi": 0x10,
  "mthi": 0x11,
  "mflo": 0x12,
  "mtlo": 0x13,
  "mult": 0x18,
  "multu": 0x19,
  "div": 0x1A,
  "divu": 0x1B,
  "add": 0x20,
  "addu": 0x21,
  "sub": 0x22,
  "subu": 0x23,
  "and": 0x24,
  "or": 0x25,
  "xor": 0x26,
  "nor": 0x27,
  "slt": 0x2A,
  "sltu": 0x2B
};