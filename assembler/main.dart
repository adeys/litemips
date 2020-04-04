import 'dart:io';

import 'src/lexer.dart';
import 'src/parser.dart';

void main(List<String> argv) {
  if (argv.length != 1) {
    print("Usage : lasm [file]");
    exit(1);
  }

  File file = new File(argv[0]);
  if (!file.existsSync()) {
    stderr.writeln("Cannot open file '${argv[0]}'.");
    exit(1);
  }

  Lexer lexer = new Lexer(file.readAsStringSync());
  lexer.tokenize();

  if (lexer.hadError) exit(1);

  Parser parser = new Parser(lexer.tokens);
  parser.parse();

  if (parser.hadError) exit(1);
}
