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
    while(!isAtEnd()) {
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
    } else if (peek().type == TokenType.T_INSTRUCTION && segment == Segment.SGT_DATA) {
      reportError("Cannot declare instruction inside of a .data segment.");
      throw new ParseError();
    }

    if (peek().type == TokenType.T_INSTRUCTION) {
      Label label = new Label(this.current.value, Segment.SGT_TEXT, this.assembly.instructions.length);
      this.assembly.addLabel(label);
      return;
    }

    if (peek().type == TokenType.T_DIRECTIVE) {
      Label label = new Label(this.current.value, Segment.SGT_DATA, this.assembly.dataSize);
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

    if (segment != Segment.SGT_DATA) {
      reportError("Cannot put directive ${this.current.lexeme} outside of a .data segment.");
      throw new ParseError();
    }

    Token token = this.current;
    switch(token.value) {
      case ".space": {
        Token amount = expect(TokenType.T_SCALAR, "Expected constat scalar expression as .space operand.");
        Directive directive = new Directive(this.current.value);
        directive.operands.add(amount.value);

        this.assembly.addDirective(directive);
        this.assembly.dataSize += amount.value;
        break;
      }
      case ".byte": {
        getData(".byte", 1);
        break;
      }
      case ".half": {
        getData(".half", 2);
        break;
      }
      case ".word": {
        getData(".word", 4);
        break;
      }
      case ".ascii":
      case ".asciiz": {
        List<Object> operands = [];
        int size = 0;
        do {
          Token operand = expect(TokenType.T_STRING, "Expected constant string as ${this.current.value} operand.");
          operands.add(operand.value);
          size += operand.value.toString().length;
        } while(matches(TokenType.T_COMMA));

        Directive directive = new Directive(this.current.value);
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

    switch(token.value) {
      case "abs":
      case "div":
      case "divu":
      case "mult":
      case "multu":
      case "neg":
      case "negu":
      case "jalr":
      case "move": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as ${token.value} first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token src = expect(TokenType.T_REGISTER, "Expected register as ${token.value} second operand.");

        Instruction instr = new Instruction(token.value, 2, InstructionType.R_TYPE);
        instr.operands = [dest, src];

        this.assembly.addInstruction(instr);
        break;
      }
      case "add":
      case "addu":
      case "and":
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
      case "sgeu":
      case "sgt":
      case "sgtu":
      case "sle":
      case "sleu":
      case "sne": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as ${token.value} first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token src = expect(TokenType.T_REGISTER, "Expected register as ${token.value} second operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token tgt = expect(TokenType.T_REGISTER, "Expected register as ${token.value} third operand.");

        Instruction instr = new Instruction(token.value, 3, InstructionType.R_TYPE);
        instr.operands = [dest, src, tgt];

        this.assembly.addInstruction(instr);
        break;
      }
      case "bge":
      case "bgeu":
      case "bgt":
      case "bgtu":
      case "ble":
      case "bleu":
      case "blt":
      case "bltu": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as ${token.value} first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token src = expect(TokenType.T_REGISTER, "Expected register as ${token.value} second operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token lbl = expect(TokenType.T_IDENTIFIER, "Expected label as ${token.value} third operand.");

        Instruction instr = new Instruction(token.value, 3, InstructionType.R_TYPE);
        instr.operands = [dest, src, lbl];

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
      case "beq":
      case "bne": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as ${token.value} first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token src = expect(TokenType.T_REGISTER, "Expected register as ${token.value} second operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token imm = expect(TokenType.T_SCALAR, "Expected constant scalar as ${token.value} third operand.");

        Instruction instr = new Instruction(token.value, 3, InstructionType.I_TYPE);
        instr.operands = [dest, src, imm];

        this.assembly.addInstruction(instr);
        break;
      }
      case "li": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as ${token.value} first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token imm = expect(TokenType.T_SCALAR, "Expected constant scalar as ${token.value} second operand.");

        Instruction instr = new Instruction(token.value, 2, InstructionType.I_TYPE);
        instr.operands = [dest, imm];

        this.assembly.addInstruction(instr);
        break;
      }
      case "bgez":
      case "bgezal":
      case "bgtz":
      case "blez":
      case "bltzal":
      case "bltz":
      case "beqz":
      case "bnez":
      case "la": {
        Token dest = expect(TokenType.T_REGISTER, "Expected register as '${token.value}' first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token lbl = expect(TokenType.T_IDENTIFIER, "Expected label as '${token.value}' second operand.");

        Instruction instr = new Instruction(token.value, 2, InstructionType.I_TYPE);
        instr.operands = [dest, lbl];

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
      case "sw": {
        Token tgt = expect(TokenType.T_REGISTER, "Expected register as '${token.value}' first operand.");
        expect(TokenType.T_COMMA, "Expected ',' between operands.");
        Token offset = new Token(TokenType.T_SCALAR, 0, 0);
        if (matches(TokenType.T_SCALAR)) {
          offset = this.current;
        }

        expect(TokenType.T_LPAREN, "Expected '(' after offset value.");
        Token src = expect(TokenType.T_REGISTER, "Expected register as '${token.value}' second operand.");
        expect(TokenType.T_RPAREN, "Expected ')' after register value.");

        Instruction instr = new Instruction(token.value, 3, InstructionType.I_TYPE);
        instr.operands = [tgt, offset, src];

        this.assembly.addInstruction(instr);
        break;
      }
      case "jr":
      case "mfhi":
      case "mflo":
      case "mthi":
      case "mtlo": {
        Token lbl = expect(TokenType.T_REGISTER, "Expected register as '${token.value}' operand.");

        Instruction instr = new Instruction(token.value, 1, InstructionType.J_TYPE);
        instr.operands = [lbl];

        this.assembly.addInstruction(instr);
        break;
      }
      case "b":
      case "j":
      case "jal": {
        Token lbl = expect(TokenType.T_IDENTIFIER, "Expected label as '${token.value}' operand.");

        Instruction instr = new Instruction(token.value, 1, InstructionType.J_TYPE);
        instr.operands = [lbl];

        this.assembly.addInstruction(instr);
        break;
      }
      case "syscall": {
        this.assembly.addInstruction(new Instruction("syscall", 0, InstructionType.J_TYPE));
        break;
      }
    }
  }

  void getData(String kind, int size) {
    List<Object> operands = [];
    do {
      Token operand = expect(TokenType.T_SCALAR, "Expected constant scalar as $kind operand.");
      operands.add(operand.value);
    } while(matches(TokenType.T_COMMA));

    Directive directive = new Directive(kind);
    directive.operands = operands;

    this.assembly.dataSize += (operands.length * size);
    this.assembly.addDirective(directive);
  }

  void synchronize() {
    advance();

    while(!isAtEnd()) {
      switch(peek().type) {
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
    stderr.writeln("[line ${peek().line}] Parse Error : Error at '${peek().lexeme}', $message");
  }

}

class ParseError {

}