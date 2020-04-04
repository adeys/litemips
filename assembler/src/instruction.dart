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
  List<Token> operands;
  InstructionType type;

  Instruction(this.name, this.opCount, this.type);
}
