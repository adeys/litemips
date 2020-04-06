import 'dart:io';

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

List<String> instructions = [
  "abs",
  "add",
  "addi",
  "addu",
  "addiu",
  "and",
  "andi",
  "div",
  "divu",
  "mul",
  "mult",
  "multu",
  "neg",
  "negu",
  "nor",
  "not",
  "or",
  "rem",
  "remu",
  "sll",
  "sllv",
  "sra",
  "srav",
  "srl",
  "srlv",
  "sub",
  "subu",
  "xor",
  "xori",
  "lui",
  "li",
  "slt",
  "sltu",
  "slti",
  "sltiu",
  "seq",
  "sge",
  "sgeu",
  "sgt",
  "sgtu",
  "sle",
  "sleu",
  "sne",
  "b",
  "beq",
  "bgez",
  "bgtz",
  "blez",
  "bltz",
  "bne",
  "beqz",
  "bge",
  "bgt",
  "ble",
  "blt",
  "bnez",
  "j",
  "jal",
  "jalr",
  "jr",
  "la",
  "lb",
  "lbu",
  "lh",
  "lhu",
  "lw",
  "sb",
  "sh",
  "sw",
  "move",
  "mfhi",
  "mflo",
  "mthi",
  "mtlo",
  "syscall",
];

List<String> directives = [
  ".text",
  ".data",
  ".ascii",
  ".asciiz",
  ".space",
  ".byte",
  ".half",
  ".word"
];

class Lexer {
  String program;
  int start = 0;
  int position = 0;
  int line = 1;
  List<Token> tokens = [];
  List<String> _escapers = ['n', 't', 'b', 'r', '\\', '"'];
  bool hadError = false;

  Lexer(this.program);

  List<Token> tokenize() {
    while (!this.isAtEnd()) {
      getNextToken();
    }

    tokens.add(new Token(TokenType.T_EOF, line, null));
    return tokens;
  }

  bool isAtEnd() {
    return position >= program.length;
  }

  void getNextToken() {
    this.start = this.position;
    String char = advance();

    if (isAlpha(char)) return getIdentifier();
    if (isDigit(char)) return getScalar(char);

    switch (char) {
      case "(":
        return addToken(TokenType.T_LPAREN, char);
      case ")":
        return addToken(TokenType.T_RPAREN, char);
      case ",":
        return addToken(TokenType.T_COMMA, char);
      case ".":
        return getDirective();
      case "\$":
        return getRegister();
      case "\"":
        return getString();
      case "-":
        return getScalar(advance(), true);
      case "#":
        {
          while (peek() != "\n" && !isAtEnd()) advance();
          break;
        }
      case "\n":
        line++;
        return;
      case " ":
      case "\t":
      case "\b":
      case "\r":
        break;
      default:
        reportError("Invalid token");
        break;
    }
  }

  void addToken(TokenType type, Object value) {
    Token token = new Token(type, line, value);
    token.lexeme = this.program.substring(this.start, this.position);
    tokens.add(token);
  }

  void getDirective() {
    while (isAlphaNum(peek())) {
      advance();
    }

    String directive = this.program.substring(this.start, this.position);
    if (!directives.contains(directive)) {
      reportError("Invalid directive $directive.");
      return;
    }

    addToken(TokenType.T_DIRECTIVE, directive);
  }

  void getRegister() {
    while (isAlphaNum(peek())) {
      advance();
    }

    String register = this.program.substring(this.start, this.position);

    if (!registers.contains(register)) {
      reportError("Invalid register name : '$register'.");
      return;
    }

    addToken(TokenType.T_REGISTER, registers.indexOf(register));
  }

  void getString() {
    StringBuffer string = new StringBuffer();
    while (peek() != '\n' && peek() != '"' && !isAtEnd()) {
      if (peek() == '\\' && _escapers.contains(peekNext())) {
        advance();
        String char;
        switch (advance()) {
          case 'n':
            char = '\n';
            break;
          case 't':
            char = '\t';
            break;
          case 'b':
            char = '\b';
            break;
          case 'r':
            char = '\r';
            break;
          case '\\':
            char = '\\';
            break;
          case '"':
            char = '"';
            break;
        }
        string.write(char);
      } else {
        string.write(advance());
      }
    }

    if (isAtEnd() || peek() == '\n') {
      reportError('Unterminated string.');
      return;
    }
    // Consume '"'
    advance();

    addToken(TokenType.T_STRING, string.toString());
  }

  void getIdentifier() {
    while (isAlphaNum(peek())) {
      advance();
    }

    TokenType type = TokenType.T_IDENTIFIER;
    String value = this.program.substring(this.start, this.position);

    if (instructions.contains(value)) {
      type = TokenType.T_INSTRUCTION;
    } else if (peek() == ":") {
      type = TokenType.T_LABEL;
      advance();
    }

    addToken(type, value);
  }

  void getScalar(String first, [bool negative = false]) {
    num value = 0;
    if (first == "0" && peek().toUpperCase() == "X") {
      advance();
      // Get hex number;
      while (isHexDigit(peek())) {
        advance();
      }
      value = int.parse(this.program.substring(this.start + 2, this.position),
          radix: 16);
    } else {
      while (isDigit(peek())) advance();
      value = int.parse(this.program.substring(this.start, this.position),
          radix: 10);
    }

    addToken(TokenType.T_SCALAR, value);
  }

  String advance() {
    return program[position++];
  }

  void reportError(String error) {
    stderr.writeln("[line $line] Syntax error : $error");
    hadError = true;
  }

  String peek() {
    return isAtEnd() ? '' : program[position];
  }

  String peekNext() {
    return isAtEnd() ? '' : program[position + 1];
  }

  bool isAlphaNum(String char) {
    return char == "_" || isAlpha(char) || isDigit(char);
  }

  bool isAlpha(String char) {
    return new RegExp("[a-zA-Z]").hasMatch(char);
  }

  bool isDigit(String char) {
    return new RegExp("[0-9]").hasMatch(char);
  }

  bool isHexDigit(String char) {
    return isDigit(char) || new RegExp("[A-Fa-f]").hasMatch(char);
  }
}
