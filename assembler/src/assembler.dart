import 'dart:typed_data';

import 'assembly.dart';
import 'instruction.dart';
import 'token.dart';

List<String> registers = [
  "\$zero",
  "\$at",
  "\$v0",
  "\$v1",
  "\$a0",
  "\$a1",
  "\$a2",
  "\$a3",
  "\$t0",
  "\$t1",
  "\$t2",
  "\$t3",
  "\$t4",
  "\$t5",
  "\$t6",
  "\$t7",
  "\$s0",
  "\$s1",
  "\$s2",
  "\$s3",
  "\$s4",
  "\$s5",
  "\$s6",
  "\$s7",
  "\$t8",
  "\$t9",
  "\$k0",
  "\$k1",
  "\$gp",
  "\$sp",
  "\$fp",
  "\$ra"
];

int getRegister(String name) {
  return registers.indexOf(name);
}

class SectionHeader {
  String name;
  int type;
  int offset;
  int size;

  SectionHeader(this.name, this.type, this.offset);
}

class Assembler {
  Assembly assembly;
  Uint8List buffer;
  int offset = 0;
  List<SectionHeader> headers = [];
  int entry = 0;
  int sha = 0;
  int strt  = 0;

  Assembler(Assembly program) {
    this.assembly = program;
    int size = assembly.instructions.length * 4 + assembly.dataSize;
    buffer = new Uint8List(size * 10);
    offset = 15; // File header length
  }

  Uint8List assemble() {
    this.resolveLabels();

    this.entry = this.offset;
    if (this.assembly.labels.containsKey("main")) {
      this.entry += this.assembly.labels["main"].address;
    }

    this.emitInstructions();
    this.emitInstructionHeader();
    this.emitStringTable();
    this.emitDataSection();
    this.emitSectionHeaders();
    this.emitFileHeader();

    return this.buffer;
  }

  void emitInstructionHeader() {
    SectionHeader header = new SectionHeader(".text", 0x01, 15);
    header.size = this.offset - 15;

    headers.add(header);
  }

  void resolveLabels() {
    for (Label label in assembly.labels.values) {
      if (label.segment == Segment.SGT_TEXT) {
        label.address *= 4;
      }
    }
  }

  void emitStringTable() {
    SectionHeader string = new SectionHeader(".string", 0x02, this.offset);

    this.emitBytes("\0".codeUnits);
    for(Directive directive in this.assembly.directives) {
      if (directive.name == ".ascii") {
        for(int i = 0; i < directive.operands.length; i++) {
          this.emitBytes(directive.operands[i].toString().codeUnits);
        }
      } else if (directive.name == ".asciiz") {
        for(int i = 0; i < directive.operands.length; i++) {
          this.emitBytes(directive.operands[i].toString().codeUnits + "\0".codeUnits);
        }
      }
    }

    this.emitBytes("\0".codeUnits);

    string.size = this.offset - string.offset;
    headers.add(string);
  }

  void emitDataSection() {
    SectionHeader data = new SectionHeader(".data", 0x04, this.offset);

    List<Directive> directives = this.assembly
        .directives
        .where((Directive direct) => direct.name != ".ascii" && direct.name != ".asciiz")
        .toList();

    Map<String, Function> map = {
      ".byte": this.emitByte,
      ".half": this.emitHalf,
      ".word": this.emitWord
    };

    directives.forEach((Directive directive) {
      for(int i = 0; i < directive.operands.length; i++) {
        map[directive.name](directive.operands[i]);
      }
    });

    data.size = this.offset - data.offset;

    headers.add(data);
  }

  void emitSectionHeaders() {
    this.sha = this.offset;
    for(SectionHeader header in this.headers) {
      this.emitHalf(0);
      this.emitByte(header.type);
      this.emitWord(header.offset);
      this.emitWord(header.size);
    }
  }

  void emitFileHeader() {
    int length = this.offset;

    this.offset = 0;
    this.emitByte(0x10);
    this.emitBytes("LEF".codeUnits);
    this.emitBytes([0x01, 0x00]); // Writes major and minor version;
    this.emitWord(this.entry);
    this.emitWord(this.sha);
    this.emitByte(headers.length);

    this.offset = length;
  }

  void emitInstructions() {
    for (Instruction instr in assembly.instructions) {
      switch (instr.name) {
        case "add":
          {
            var tgt = instr.operands[2].value;
            if (instr.operands[2].type == TokenType.T_SCALAR) {
              // addi $at, $zero, immed
              tgt = "\$at";
              this.emitImmediate(0x08, getRegister("\$zero"), getRegister(tgt),
                  instr.operands[2].value);
            }
            this.emitSpecial(0x20, getRegister(instr.operands[1].value),
                getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
            break;
          }
        case "addi": {
          this.emitImmediate(0x08, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "addiu": {
          this.emitImmediate(0x09, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "addu": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x21, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "and": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x08, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x24, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "andi": {
          this.emitImmediate(0x0C, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "div": {
          this.emitSpecial(0x1A, getRegister(instr.operands[0].value),
              getRegister(instr.operands[1].value), 0x00, 0x00);
          break;
        }
        case "divu": {
          this.emitSpecial(0x1B, getRegister(instr.operands[0].value),
              getRegister(instr.operands[1].value), 0x00, 0x00);
          break;
        }
        case "mult": {
          this.emitSpecial(0x18, getRegister(instr.operands[0].value),
              getRegister(instr.operands[1].value), 0x00, 0x00);
          break;
        }
        case "multu": {
          this.emitSpecial(0x19, getRegister(instr.operands[0].value),
              getRegister(instr.operands[1].value), 0x00, 0x00);
          break;
        }
        case "neg": {
          // neg rd, rs -> sub rd, zero, src
          this.emitSpecial(0x22, getRegister("\$zero"), getRegister(instr.operands[1].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "negu": {
          // negu rd, rs -> sub rd, zero, src
          this.emitSpecial(0x23, getRegister("\$zero"), getRegister(instr.operands[1].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "nor": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x27, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "or": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x08, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x25, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "ori": {
          this.emitImmediate(0x0D, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "rem": {
          // rem rd, rs, rt -> div rs, rt ; mfhi rd
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x1A, getRegister(instr.operands[1].value),
              getRegister(tgt), 0x00, 0x00);
          this.emitSpecial(0x10, 0x00, 0x00, getRegister(instr.operands[1].value), 0x00);
          break;
        }
        case "remu": {
          // remu rd, rs, rt -> divu rs, rt ; mfhi rd
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x1B, getRegister(instr.operands[1].value),
              getRegister(tgt), 0x00, 0x00);
          this.emitSpecial(0x10, 0x00, 0x00, getRegister(instr.operands[1].value), 0x00);
          break;
        }
        case "sll": {
          this.emitSpecial(0x00, 0x00, getRegister(instr.operands[1].value),
              getRegister(instr.operands[0].value), instr.operands[2].value);
          break;
        }
        case "sllv": {
          this.emitSpecial(0x04, getRegister(instr.operands[2].value),
              getRegister(instr.operands[1].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "sra": {
          this.emitSpecial(0x03, 0x00, getRegister(instr.operands[1].value),
              getRegister(instr.operands[0].value), instr.operands[2].value);
          break;
        }
        case "srav": {
          this.emitSpecial(0x07, getRegister(instr.operands[1].value),
              getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "srl": {
          this.emitSpecial(0x02, 0x00, getRegister(instr.operands[1].value),
              getRegister(instr.operands[0].value), instr.operands[2].value);
          break;
        }
        case "srlv": {
          this.emitSpecial(0x06, getRegister(instr.operands[1].value),
              getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "sub": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addi $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x08, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x22, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "subu": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x23, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "xor": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x26, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "xori": {
          this.emitImmediate(0x0E, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "li": {
          this.emitImmediate(0x09, getRegister("\$zero"), getRegister(instr.operands[0].value),
              instr.operands[1].value);
          break;
        }
        case "slt": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x2A, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "sltu": {
          var tgt = instr.operands[2].value;
          if (instr.operands[2].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[2].value);
          }
          this.emitSpecial(0x2B, getRegister(instr.operands[1].value),
              getRegister(tgt), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "slti": {
          this.emitImmediate(0x0A, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "sltiu": {
          this.emitImmediate(0x0B, getRegister(instr.operands[1].value), getRegister(instr.operands[0].value),
              instr.operands[2].value);
          break;
        }
        case "b":
        case "j":{
          Token label = instr.operands[0];
          int address;
          if (label.type == TokenType.T_IDENTIFIER) {
            if(!this.assembly.labels.containsKey(label.value)) {
              throw new AssemblerError(label, "Undefined label '${label.value}'.");
            }

            address = this.assembly.labels[label.value].address >> 2;
          } else {
            address = ((label.value as int) >> 2) & 0x03FFFFFF;
          }

          this.emitJInstruction(0x02, address);
          break;
        }
        case "beq": {
          Token label = instr.operands[2];
          int address;
          if (label.type == TokenType.T_IDENTIFIER) {
            if(!this.assembly.labels.containsKey(label.value)) {
              throw new AssemblerError(label, "Undefined label '${label.value}'.");
            }

            address = this.assembly.labels[label.value].address >> 2;
          } else {
            address = ((label.value as int) >> 2) & 0x03FFFFFF;
          }

          String tgt = instr.operands[1].value;
          if (instr.operands[1].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[1].value);
          }

          this.emitImmediate(0x04, getRegister(instr.operands[0].value), getRegister(tgt), address - this.offset - 1);
          break;
        }
        case "bne": {
          Token label = instr.operands[2];
          int address;
          if (label.type == TokenType.T_IDENTIFIER) {
            if(!this.assembly.labels.containsKey(label.value)) {
              throw new AssemblerError(label, "Undefined label '${label.value}'.");
            }

            address = this.assembly.labels[label.value].address >> 2;
          } else {
            address = ((label.value as int) >> 2) & 0x03FFFFFF;
          }

          String tgt = instr.operands[1].value;
          if (instr.operands[1].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[1].value);
          }

          this.emitImmediate(0x05, getRegister(instr.operands[0].value), getRegister(tgt), address - this.offset - 1);
          break;
        }
        case "blt": {
          // blt rs, rt, label -> slt $at, rs, rt ; bne $at, $zero, label;

          // slt $at, rs, rt
          var tgt = instr.operands[1].value;
          if (instr.operands[1].type == TokenType.T_SCALAR) {
            // addiu $at, $zero, immed
            tgt = "\$at";
            this.emitImmediate(0x09, getRegister("\$zero"), getRegister(tgt),
                instr.operands[1].value);
          }

          this.emitSpecial(0x2A, getRegister(instr.operands[0].value),
              getRegister(tgt), getRegister("\$at"), 0x00);

          // bne $at, $zero, label
          int address;
          var label = instr.operands[2];
          if (label.type == TokenType.T_IDENTIFIER) {
            if(!this.assembly.labels.containsKey(label.value)) {
              throw new AssemblerError(label, "Undefined label '${label.value}'.");
            }

            address = this.assembly.labels[label.value].address >> 2;
          } else {
            address = ((label.value as int) >> 2) & 0x03FFFFFF;
          }
          this.emitImmediate(0x05, getRegister("\$at"), getRegister("\$zero"), address - this.offset - 1);
          break;
        }
        case "jal": {
          Token label = instr.operands[0];
          int address;
          if (label.type == TokenType.T_IDENTIFIER) {
            if(!this.assembly.labels.containsKey(label.value)) {
              throw new AssemblerError(label, "Undefined label '${label.value}'.");
            }

            address = this.assembly.labels[label.value].address >> 2;
          } else {
            address = ((label.value as int) >> 2) & 0x03FFFFFF;
          }

          this.emitJInstruction(0x03, address);
          break;
        }
        case "jr": {
          this.emitSpecial(0x08, getRegister(instr.operands[0].value), 0x00, 0x00, 0x00);
          break;
        }
        case "jalr": {
          this.emitSpecial(0x09, getRegister(instr.operands[0].value), 0x00, getRegister(instr.operands[1].value), 0x00);
          break;
        }
        case "la": {
          Token label = instr.operands[1];
          int address;
          if(!this.assembly.labels.containsKey(label.value)) {
            throw new AssemblerError(label, "Undefined label '${label.value}'.");
          }

          address = this.assembly.labels[label.value].address;
          // addiu $rd, $gp, address
          this.emitImmediate(0x09, getRegister("\$gp"), getRegister(instr.operands[0].value), address);
          break;
        }
        case "lb": {
          this.emitImmediate(0x20, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "lbu": {
          this.emitImmediate(0x24, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "lh": {
          this.emitImmediate(0x21, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "lhu": {
          this.emitImmediate(0x25, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "lw": {
          this.emitImmediate(0x23, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "sb": {
          this.emitImmediate(0x28, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "sh": {
          this.emitImmediate(0x29, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "sw": {
          this.emitImmediate(0x2b, getRegister(instr.operands[2].value), getRegister(instr.operands[0].value), instr.operands[1].value);
          break;
        }
        case "move": {
          this.emitSpecial(0x20, getRegister("\$zero"), getRegister(instr.operands[1].value), getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "mfhi": {
          this.emitSpecial(0x10, 0x00, 0x00, getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "mflo": {
          this.emitSpecial(0x12, 0x00, 0x00, getRegister(instr.operands[0].value), 0x00);
          break;
        }
        case "mthi": {
          this.emitSpecial(0x11, getRegister(instr.operands[0].value), 0x00, 0x00, 0x00);
          break;
        }
        case "mfhi": {
          this.emitSpecial(0x10, getRegister(instr.operands[0].value), 0x00, 0x00, 0x00);
          break;
        }
        case "syscall": {
          this.emitSpecial(0x0C, 0x00, 0x00, 0x00, 0x00);
          break;
        }
        default:
          throw new AssemblerError(null, "Instruction '${instr.name}' is not yet supported.");
      }
    }
  }

  void emitJInstruction(int code, int immediate) {
    int instr = (code << 26) | immediate;
    this.emitWord(instr);
  }

  void emitSpecial(int code, int rs, int rt, int rd, int shmt) {
    int instr = (0x00 << 26) |
        (rs << 21) |
        (rt << 16) |
        (rd << 11) |
        (shmt << 6) |
        (code & 0x3F);
    this.emitWord(instr);
  }

  void emitImmediate(int code, int rs, int rt, int immed) {
    int instr = (code << 26) | (rs << 21) | (rt << 16) | (immed & 0xFFFF);
    this.emitWord(instr);
  }

  void emitByte(int byte) {
    this.buffer[offset++] = byte;
  }

  void emitBytes(List<int> bytes) {
    this.buffer.setAll(offset, bytes);
    offset += bytes.length;
  }

  void emitHalf(int half) {
    emitByte(half >> 0x08);
    emitByte(half);
  }

  void emitWord(int word) {
    emitByte(word >> 0x18);
    emitByte(word >> 0x10);
    emitByte(word >> 0x08);
    emitByte(word);
  }
}

class AssemblerError {
  String message;
  Token token;

  AssemblerError(this.token, this.message);
}