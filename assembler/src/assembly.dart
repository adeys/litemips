import 'instruction.dart';

enum Segment {
  SGT_TEXT,
  SGT_DATA
}

class Label {
  String name;
  int address;
  Segment segment;

  Label(this.name, this.segment, this.address);
}

class Directive {
  String name;
  List<Object> operands = [];

  Directive(this.name);
}

class Assembly {
  List<Instruction> instructions = [];
  List<Directive> directives = [];
  List<Label> labels = [];
  int dataSize = 0;

  void addInstruction(Instruction instruction) {
    instructions.add(instruction);
  }

  void addLabel(Label label) {
    labels.add(label);
  }

  void addDirective(Directive directive) {
    directives.add(directive);
  }
}