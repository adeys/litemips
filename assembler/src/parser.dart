import 'dart:io';

import 'assembly.dart';
import 'instruction.dart';
import 'token.dart';

class Parser {
  List<Token> tokens;
  int offset = 0;
  Assembly assembly = new Assembly();
  Segment segment = Segment.SGT_TEXT;
  Token current;
  bool hadError = false;

  Parser(this.tokens);

  void parse() {
    while (!isAtEnd()) {
      getDeclaration();
    }
  }

  void getDeclaration() {
    try {
      if (matches(TokenType.T_LABEL)) return getLabel();
      if (matches(TokenType.T_DIRECTIVE)) return getDirective();

      getInstruction();
    } on ParseError catch (_) {
      synchronize();
      return;
    }
  }

  void getLabel() {
    if ((peek().type == TokenType.T_DIRECTIVE && segment != Segment.SGT_DATA)) {
      reportError("Cannot declare data label outside of a .data segment.");
      throw new ParseError();
    } else if (peek().type == TokenType.T_INSTRUCTION &&
        segment == Segment.SGT_DATA) {
      reportError("Cannot declare instruction inside of a .data segment.");
      throw new ParseError();
    }

    if (peek().type == TokenType.T_INSTRUCTION) {
      Label label = new Label(this.current.value, Segment.SGT_TEXT,
          this.assembly.instructions.length);
      this.assembly.addLabel(label);
      return;
    }

    if (peek().type == TokenType.T_DIRECTIVE) {
      Label label = new Label(
          this.current.value, Segment.SGT_DATA, this.assembly.dataSize);
      this.assembly.addLabel(label);
      return;
    }

    reportError("Expected an instruction or a directive.");
  }

  void getDirective() {
    if (this.current.value == ".text") {
      segment = Segment.SGT_TEXT;
      return;
    } else if (this.current.value == ".data") {
      segment = Segment.SGT_DATA;
      return;
    }

    if (this.current.value == ".entry") {
      this.assembly.entryPoint = this.expect(TokenType.T_IDENTIFIER, "Expected label as .entry directive's operand.").value;
      return;
    }

    if (segment != Segment.SGT_DATA) {
      reportError(
          "Cannot put directive ${this.current.lexeme} outside of a .data segment.");
      throw new ParseError();
    }

    Token token = this.current;
    switch (token.value) {
      case ".space":
        {
          Token amount = expect(TokenType.T_SCALAR,
              "Expected constat scalar expression as .space operand.");
          Directive directive = new Directive(token.value);
          directive.operands.add(amount.value);

          this.assembly.addDirective(directive);
          this.assembly.dataSize += amount.value;
          break;
        }
      case ".byte":
        {
          getData(".byte", 1);
          break;
        }
      case ".half":
        {
          getData(".half", 2);
          break;
        }
      case ".word":
        {
          getData(".word", 4);
          break;
        }
      case ".ascii":
      case ".asciiz":
        {
          List<Object> operands = [];
          int size = 0;
          do {
            Token operand = expect(TokenType.T_STRING,
                "Expected constant string as ${token.value} operand.");
            operands.add(operand.value);
            size += operand.value.toString().length;
          } while (matches(TokenType.T_COMMA));

          Directive directive = new Directive(token.value);
          directive.operands = operands;

          if (token.value == ".asciiz") size += operands.length;
          this.assembly.dataSize += size;
          this.assembly.addDirective(directive);
          break;
        }
    }
  }

  void getInstruction() {
    Token token = expect(TokenType.T_INSTRUCTION, "Expected an instruction.");

    switch (token.value) {
      case "abs":
      case "div":
      case "divu":
      case "mult":
      case "multu":
      case "neg":
      case "negu":
      case "not":
      case "jalr":
      case "move":
        {
          Token rs = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token rt = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' second operand.");

          Instruction instr =
              new Instruction(token.value, 2, InstructionType.R_TYPE);
          instr.rs = rs;
          instr.rt = rt;

          this.assembly.addInstruction(instr);
          break;
        }
      case "add":
      case "addu":
      case "and":
      case "mul":
      case "nor":
      case "or":
      case "rem":
      case "remu":
      case "sllv":
      case "srav":
      case "srlv":
      case "sub":
      case "subu":
      case "xor":
      case "slt":
      case "sltu":
      case "seq":
      case "sge":
      case "sgt":
      case "sle":
      case "sne":
        {
          Token dest = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token src = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' second operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          if (!matches(TokenType.T_REGISTER) && !matches(TokenType.T_SCALAR)) {
            reportError(
                "Expected register or constant scalar as '${token.value}' third operand.");
            throw new ParseError();
          }

          Token tgt = this.current;

          Instruction instr =
              new Instruction(token.value, 3, InstructionType.R_TYPE);
          instr.operands = [dest, src, tgt];
          instr.rs = src;
          instr.rd = dest;
          instr.rt = tgt;

          this.assembly.addInstruction(instr);
          break;
        }
      case "beq":
      case "bne":
      case "bge":
      case "bgt":
      case "ble":
      case "blt":
        {
          Token dest = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          if (!matches(TokenType.T_REGISTER) && !matches(TokenType.T_SCALAR)) {
            reportError(
                "Expected register or constant scalar as '${token.value}' second operand.");
            throw new ParseError();
          }
          Token src = this.current;
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token lbl = expect(TokenType.T_IDENTIFIER,
              "Expected label as '${token.value}' third operand.");

          Instruction instr =
              new Instruction(token.value, 3, InstructionType.R_TYPE);
          instr.rs = dest;
          instr.rt = src;
          instr.immed = lbl;

          this.assembly.addInstruction(instr);
          break;
        }
      case "addi":
      case "addiu":
      case "andi":
      case "ori":
      case "sll":
      case "sra":
      case "srl":
      case "xori":
      case "slti":
      case "sltiu":
        {
          Token dest = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token src = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' second operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token imm = expect(TokenType.T_SCALAR,
              "Expected constant scalar as '${token.value}' third operand.");

          Instruction instr =
              new Instruction(token.value, 3, InstructionType.I_TYPE);
          instr.rs = src;
          instr.rt = dest;
          instr.immed = imm;

          this.assembly.addInstruction(instr);
          break;
        }
      case "lui":
      case "li":
        {
          Token dest = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token imm = expect(TokenType.T_SCALAR,
              "Expected constant scalar as '${token.value}' second operand.");

          Instruction instr =
              new Instruction(token.value, 2, InstructionType.I_TYPE);
          instr.operands = [dest, imm];
          instr.rt = dest;
          instr.immed = imm;

          this.assembly.addInstruction(instr);
          break;
        }
      case "bgez":
      case "bgtz":
      case "blez":
      case "bltz":
      case "beqz":
      case "bnez":
      case "la":
        {
          Token dest = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token lbl = expect(TokenType.T_IDENTIFIER,
              "Expected label as '${token.value}' second operand.");

          Instruction instr = new Instruction(token.value, 2, InstructionType.I_TYPE);
          instr.rt = dest;
          instr.immed = lbl;

          this.assembly.addInstruction(instr);
          break;
        }
      case "lb":
      case "lbu":
      case "lh":
      case "lhu":
      case "lw":
      case "sb":
      case "sh":
      case "sw":
        {
          Token tgt = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' first operand.");
          expect(TokenType.T_COMMA, "Expected ',' between operands.");
          Token offset = new Token(TokenType.T_SCALAR, 0, 0);
          if (matches(TokenType.T_SCALAR)) {
            offset = this.current;
          }

          expect(TokenType.T_LPAREN, "Expected '(' after offset value.");
          Token src = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' second operand.");
          expect(TokenType.T_RPAREN, "Expected ')' after register value.");

          Instruction instr =
              new Instruction(token.value, 3, InstructionType.I_TYPE);
          instr.rs = src;
          instr.rt = tgt;
          instr.immed = offset;

          this.assembly.addInstruction(instr);
          break;
        }
      case "jr":
      case "mfhi":
      case "mflo":
      case "mthi":
      case "mtlo":
        {
          Token lbl = expect(TokenType.T_REGISTER,
              "Expected register as '${token.value}' operand.");

          Instruction instr =
              new Instruction(token.value, 1, InstructionType.J_TYPE);
          if (token.value.toString().startsWith("mf")) {
            instr.rd = lbl;
          } else {
            instr.rs = lbl;
          }

          this.assembly.addInstruction(instr);
          break;
        }
      case "b":
      case "j":
      case "jal":
        {
          if (!matches(TokenType.T_IDENTIFIER) && !matches(TokenType.T_SCALAR)) {
            reportError("Expected label or constant address as '${token.value}' operand.");
            throw new ParseError();
          }
          Token lbl = this.current;

          Instruction instr =
              new Instruction(token.value, 1, InstructionType.J_TYPE);
          instr.immed = lbl;

          this.assembly.addInstruction(instr);
          break;
        }
      case "syscall":
        {
          this.assembly.addInstruction(
              new Instruction("syscall", 0, InstructionType.J_TYPE));
          break;
        }
    }
  }

  void getData(String kind, int size) {
    List<Object> operands = [];
    do {
      Token operand = expect(
          TokenType.T_SCALAR, "Expected constant scalar as $kind operand.");
      operands.add(operand.value);
    } while (matches(TokenType.T_COMMA));

    Directive directive = new Directive(kind);
    directive.operands = operands;

    this.assembly.dataSize += (operands.length * size);
    this.assembly.addDirective(directive);
  }

  void synchronize() {
    advance();

    while (!isAtEnd()) {
      switch (peek().type) {
        case TokenType.T_INSTRUCTION:
        case TokenType.T_DIRECTIVE:
        case TokenType.T_LABEL:
          return;
        default:
          advance();
      }
    }
  }

  bool isAtEnd() {
    return this.tokens[this.offset].type == TokenType.T_EOF;
  }

  Token peek() {
    return this.tokens[this.offset];
  }

  Token advance() {
    return this.current = this.tokens[this.offset++];
  }

  bool matches(TokenType type) {
    if (this.peek().type == type) {
      advance();
      return true;
    }

    return false;
  }

  Token expect(TokenType type, String errMsg) {
    if (peek().type == type) {
      return advance();
    }

    reportError(errMsg);
    throw new ParseError();
  }

  void reportError(String message) {
    hadError = true;
    stderr.writeln(
        "[line ${peek().line}] Parse Error : Error at '${peek().lexeme}', $message");
  }
}

class ParseError {}
