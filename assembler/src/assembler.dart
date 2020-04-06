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
  int address = 0;
  List<SectionHeader> headers = [];
  int entry = 0;
  int sha = 0;
  int strt  = 0;
  List<int> relocations = [];
  // CPU dependant
  final int DATA_TOP = 0x07FFFF;

  Assembler(Assembly program) {
    this.assembly = program;
    int size = assembly.instructions.length * 4 + assembly.dataSize;
    buffer = new Uint8List(size * 10);
    offset = 15; // File header length
  }

  Uint8List assemble() {
    this.createRelocationTable();
    this.resolveLabels();

    this.entry = this.offset;
    if (this.assembly.labels.containsKey("main")) {
      this.entry += this.assembly.labels["main"].address;
    }

    this.emitInstructions();
    this.emitInstructionHeader();
    this.emitDataSection();
    this.emitStringTable();
    this.emitSectionHeaders();
    this.emitFileHeader();

    return this.buffer;
  }

  void emitInstructionHeader() {
    SectionHeader header = new SectionHeader(".text", 0x01, 15);
    header.size = this.offset - 15;

    headers.add(header);
  }

  void createRelocationTable() {
    int address = 0;
    for (var i = 0; i < this.assembly.instructions.length; ++i) {
      Instruction instr = this.assembly.instructions[i];
      this.relocations.add(address);
      address += 4;

      switch(instr.name) {
        case "add":
        case "addu":
        case "and":
        case "nor":
        case "or":
        case "sub":
        case "subu":
        case "xor":
        case "slt":
        case "sltu":
        case "beq":
        case "bne": {
          address += instr.rt.type == TokenType.T_SCALAR ? 4 : 0;
          break;
        }
        case "blez":
        case "bltz":
        case "bgtz":
        case "bgez":
        case "blt":
        case "bge":
        case "bgt":
        case "ble":
        case "sge":
        case "sgt":
        case "rem":
        case "remu": {
          address += instr.rt.type == TokenType.T_SCALAR ? 8 : 4;
          break;
        }
        case "mul":
        case "abs": {
          address += 8;
          break;
        }
        case "la":
        case "li": {
          address += 4;
          break;
        }
        default:
          break;
      }
    }
  }

  void resolveLabels() {
    for (Label label in assembly.labels.values) {
      if (label.segment == Segment.SGT_TEXT) {
        label.address = this.relocations[label.address];
      }
    }
  }

  void emitStringTable() {
    SectionHeader string = new SectionHeader(".string", 0x02, this.offset);

    this.emitByte(0);
    this.emitByte(0);

    string.size = this.offset - string.offset;
    headers.add(string);
  }

  void emitDataSection() {
    SectionHeader data = new SectionHeader(".data", 0x04, this.offset);

    Map<String, Function> map = {
      ".byte": this.emitByte,
      ".half": this.emitHalf,
      ".word": this.emitWord
    };

    for (Directive directive in this.assembly.directives) {
      switch(directive.name) {
        case ".byte":
        case ".half":
        case ".word": {
          for(int i = 0; i < directive.operands.length; i++) {
            map[directive.name](directive.operands[i]);
          }
          break;
        }
        case ".ascii": {
          for(int i = 0; i < directive.operands.length; i++) {
            this.emitBytes(directive.operands[i].toString().codeUnits);
          }
          break;
        }
        case ".asciiz": {
          for(int i = 0; i < directive.operands.length; i++) {
            this.emitBytes(directive.operands[i].toString().codeUnits + [0]);
          }
          break;
        }
      }
    }

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
        case "abs": {
          this.emitSpecial("addu", 0x00, instr.rt.value, instr.rs.value, 0x00);
          this.emitImmediate("rsi", instr.rt.value, OpCodes["bgez"], 0x08 >> 2);
          this.emitSpecial("sub", 0x00, instr.rt.value, instr.rs.value, 0x00);
          break;
        }
        case "add":
        case "addu":
        case "and":
        case "nor":
        case "or":
        case "sub":
        case "subu":
        case "xor":
        case "slt":
        case "sltu": {
          int rt = this._getRt(instr.rt);
          this.emitSpecial(instr.name, instr.rs.value, rt, instr.rd.value, 0x00);
          break;
        }
        case "mul": {
          int rt = this._getRt(instr.rt);
          this.emitSpecial("mult", instr.rs.value, rt, 0x00, 0x00);
          this.emitSpecial("mflo", 0x00, 0x00, instr.rd.value, 0x00);
          break;
        }
        case "addi":
        case "addiu":
        case "andi":
        case "ori":
        case "xori":
        case "slti":
        case "sltiu": {
          this.emitImmediate(instr.name, instr.rs.value, instr.rt.value, instr.immed.value);
          break;
        }
        case "div":
        case "divu":
        case "mult":
        case "multu": {
          this.emitSpecial(instr.name, instr.rs.value, instr.rt.value, 0x00, 0x00);
          break;
        }
        case "neg":
        case "negu": {
          this.emitSpecial(instr.name.endsWith("u") ? "subu" : "sub", getRegister("\$zero"), instr.rt.value, instr.rs.value, 0x00);
          break;
        }
        case "not": {
          this.emitSpecial("nor", 0x00, instr.rs.value, instr.rt.value, 0x00);
          break;
        }
        case "rem":
        case "remu": {
          // remu rd, rs, rt -> divu rs, rt ; mfhi rd
          int rt = this._getRt(instr.rt);
          this.emitSpecial(instr.name.endsWith("u") ? "divu" : "div", instr.rs.value, rt, 0x00, 0x00);
          this.emitSpecial("mfhi", 0x00, 0x00, instr.rd.value, 0x00);
          break;
        }
        case "sll":
        case "sra":
        case "srl": {
          this.emitSpecial(instr.name, 0x00, instr.rs.value, instr.rt.value, instr.immed.value);
          break;
        }
        case "sllv":
        case "srav":
        case "srlv": {
          this.emitSpecial(instr.name, instr.rt.value, instr.rs.value, instr.rd.value, 0x00);
          break;
        }
        case "lui": {
          this.emitImmediate("lui", 0x00, instr.rt.value, instr.immed.value);
          break;
        }
        case "li": {
          this.emitImmediate("lui", 0x00, getRegister("\$at"), (instr.immed.value as int) >> 16);
          this.emitImmediate("ori", getRegister("\$at"), instr.rt.value, (instr.immed.value as int));
          break;
        }
        case "b":
        case "j":
        case "jal": {
          int address = this._getAddress(instr.immed, true);

          this.emitJInstruction(OpCodes[instr.name == "b" ? "j" : instr.name], address);
          break;
        }
        case "sge": {
          int rt = this._getRt(instr.rt);

          this.emitSpecial("slt", instr.rs.value, rt, getRegister("\$at"), 0x00);
          this.emitSpecial("xor", 0x00, getRegister("\$at"), instr.rd.value, 0x00);
          break;
        }
        case "sgt": {
          int rt = this._getRt(instr.rt);

          this.emitSpecial("sub", instr.rs.value, rt, getRegister("\$at"), 0x00);
          this.emitSpecial("slt", 0x00, getRegister("\$at"), instr.rd.value, 0x00);
          break;
        }
        case "beq":
        case "bne": {
          int address = this._getAddress(instr.immed);
          int rt = this._getRt(instr.rt);

          this.emitImmediate(instr.name, instr.rs.value, rt, address);
          break;
        }
        case "beqz":
        case "bnez": {
          int address;
          Token label = instr.immed;
          if(!this.assembly.labels.containsKey(label.value)) {
            throw new AssemblerError(label, "Undefined label '${label.value}'.");
          }

          address = this.resolveLabelAddr(this.assembly.labels[label.value].address) >> 2;

          this.emitImmediate(instr.name.substring(0, 2), 0x00, instr.rt.value, address);
          break;
        }
        case "blt":
        case "bge": {
          int rt = this._getRt(instr.rt);
          int address = this._getAddress(instr.immed);

          this.emitSpecial("slt", instr.rs.value, rt, getRegister("\$at"), 0x00);
          this.emitImmediate(instr.name == "blt" ? "bne" : "beq", getRegister("\$at"), 0x00, address);
          break;
        }
        case "bgt":
        case "ble": {
          int rt = this._getRt(instr.rt);
          int address = this._getAddress(instr.immed);

          this.emitSpecial("sub", instr.rs.value, rt, getRegister("\$at"), 0x00);
          this.emitImmediate(instr.name + "z", getRegister("\$at"), 0x00, address);
          break;
        }
        case "blez":
        case "bgtz": {
          int address = this._getAddress(instr.immed);

          this.emitImmediate(instr.name, instr.rt.value, 0x00, address);
          break;
        }
        case "bgez":
        case "bltz": {
          int address;
          Token label = instr.immed;
          if(!this.assembly.labels.containsKey(label.value)) {
            throw new AssemblerError(label, "Undefined label '${label.value}'.");
          }

          address = this.resolveLabelAddr(this.assembly.labels[label.value].address) >> 2;

          this.emitImmediate("rsi", instr.rt.value, OpCodes[instr.name], address);
          break;
        }
        case "jr": {
          this.emitSpecial("jr", instr.rs.value, 0x00, 0x00, 0x00);
          break;
        }
        case "jalr": {
          this.emitSpecial("jalr", instr.rt.value, 0x00, instr.rs.value, 0x00);
          break;
        }
        case "la": {
          Token label = instr.immed;
          int address;
          if(!this.assembly.labels.containsKey(label.value)) {
            throw new AssemblerError(label, "Undefined label '${label.value}'.");
          }

          address = DATA_TOP + this.assembly.labels[label.value].address;
          // ori $rd, $gp, address
          this.emitImmediate("lui", 0x00, getRegister("\$at"), address >> 16);
          this.emitImmediate("ori", getRegister("\$at"), instr.rt.value, address);
          break;
        }
        case "lb":
        case "lbu":
        case "lh":
        case "lhu":
        case "lw":
        case "sb":
        case "sh":
        case "sw": {
          this.emitImmediate(instr.name, instr.rs.value, instr.rt.value, instr.immed.value);
          break;
        }
        case "move": {
          this.emitSpecial("add", 0x00, instr.rt.value, instr.rs.value, 0x00);
          break;
        }
        case "mfhi":
        case "mflo": {
          this.emitSpecial(instr.name, 0x00, 0x00, instr.rd.value, 0x00);
          break;
        }
        case "mthi":
        case "mtlo": {
          this.emitSpecial(instr.name, instr.rs.value, 0x00, 0x00, 0x00);
          break;
        }
        case "syscall": {
          this.emitSpecial("syscall", 0x00, 0x00, 0x00, 0x00);
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
    this.address += 4;
  }

  void emitSpecial(String code, int rs, int rt, int rd, int shmt) {
    int instr = (0x00 << 26) |
        (rs << 21) |
        (rt << 16) |
        (rd << 11) |
        (shmt << 6) |
        (OpCodes[code] & 0x3F);
    this.emitWord(instr);
    this.address += 4;
  }

  void emitImmediate(String code, int rs, int rt, int immed) {
    int instr = (OpCodes[code] << 26) | (rs << 21) | (rt << 16) | (immed & 0xFFFF);
    this.emitWord(instr);
    this.address += 4;
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

  int resolveLabelAddr(int label) {
    return label - this.address;
  }

  int _getAddress(Token label, [bool absolute = false]) {
    if (label.type == TokenType.T_IDENTIFIER) {
      if(!this.assembly.labels.containsKey(label.value)) {
        throw new AssemblerError(label, "Undefined label '${label.value}'.");
      }

      int address = absolute
          ? this.assembly.labels[label.value].address
          : this.resolveLabelAddr(this.assembly.labels[label.value].address);
      return address >> 2;
    }

    return ((label.value as int) >> 2) & 0x03FFFFFF;
  }

  int _getRt(Token token) {
    if (token.type == TokenType.T_SCALAR) {
      // ori $at, $zero, immed
      int rt = getRegister("\$at");
      this.emitImmediate("ori", 0x00, rt, token.value);
      return rt;
    }

    return token.value;
  }
}

class AssemblerError {
  String message;
  Token token;

  AssemblerError(this.token, this.message);
}