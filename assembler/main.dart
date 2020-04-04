import 'dart:io';
import 'dart:typed_data';

import 'src/assembler.dart';
import 'src/lexer.dart';
import 'src/parser.dart';

void main(List<String> argv) {
  if (argv.length != 3) {
    print("Usage : lasm [file] -o [output]");
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

  Assembler assembler = new Assembler(parser.assembly);
  Uint8List program = assembler.assemble();

  File out = new File(argv[2]);
  out.createSync();
  out.writeAsBytesSync(program);
}
