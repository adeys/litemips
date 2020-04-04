enum TokenType {
  // Punctuations
  T_LPAREN,
  T_RPAREN,
  T_COMMA,

  // Constants
  T_STRING,
  T_SCALAR,

  // Keywords
  T_IDENTIFIER,
  T_INSTRUCTION,
  T_DIRECTIVE,
  T_REGISTER,
  T_LABEL,

  T_EOF
}

class Token {
  TokenType type;
  int line;
  String lexeme;
  Object value;

  Token(this.type, this.line, this.value);

  @override
  String toString() {
    return type.toString() + " " + value.toString();
  }
}
