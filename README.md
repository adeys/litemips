Lite Mips Simulator (LMS)
--------------------------

Lite Mips Simulator is a MIPS subset implementation.

## Registers
**LMS** has :
- 32 32-bit sized general-purpose registers
- 3 special registers

### General purpose registers
They are named following two differents conventions: 
- Either with their associated number (0 - 31)
- Either with their special use cases

| Register Number | Alternative Name | Description |
|  :-----------:  |  :------------:  | :---------: |
|   0   | $zero | the value 0 |
|   1   | $at  | (assembler temporary) reserved by the assembler |
|  2-3  | $v0 - $v1 | (values) from expression evaluation and function results |
|  4-7  | $a0 - $a3 | (arguments) First four parameters for subroutine. Not preserved across procedure calls |
|  8-15 | $t0 - $t7 | (temporaries) Caller saved if needed. Subroutines can use w/out saving. Not preserved across procedure calls |
| 16-23 | $s0 - $s7 | (saved values) - Callee saved. A subroutine using one of these must save original and restore it before exiting. Preserved across procedure calls |
| 24-25 | $t8 - $t9 | (temporaries) Caller saved if needed. Subroutines can use w/out saving. These are in addition to $t0 - $t7 above. Not preserved across procedure calls. |
| 26-27 | $k0 - $k1 | reserved for use by the interrupt/trap handler
|   28  |    $gp    | global pointer. Points to the middle of the 64K block of memory in the static data segment. |
|   29  |    $sp    | stack pointer Points to last location on the stack. |
|   30  |  $s8/$fp  | saved value / frame pointer Preserved across procedure calls |
|   31  |    $ra    | return address |


### Special purpose registers
- **HI** register : Holds the high-part of a multiplication operation and the quotient of a division operation
- **LO** register : Holds the low-part of a multiplication operation and the remainder of a division operation
- **PC** register : Program Counter register

## Instructions
Instructions within **LMS** are 32bit long with the first 6bits reserved to **OpCode**.
Following formats are possible

- R-type formats
~~~
31       25     20     15     10     5           0
+--------+------+------+------+------+-----------+
| OpCode |  Rs  |  Rt  |  Rd  |  SA  | Func Code |
+--------+------+------+------+------+-----------+
~~~

- I-type formats 
~~~
31       25     20     15                        0
+--------+------+------+-------------------------+
| OpCode |  Rs  |  Rt  | 2’s complement constant |
+--------+------+------+-------------------------+
~~~

- J-type formats 
~~~
31       25                                      0
+--------+---------------------------------------+
| OpCode |               Jump target             |
+--------+---------------------------------------+
~~~

## Instructions

Following instructions are supported

- Arithmetic and Logical Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|  add  |  100000  | f $d, $s, $t | $d = $s + $t |
| addu  |  100001  | f $d, $s, $t | $d = $s + $t |
| addi  |  001000  | f $d, $s, i  | $d = $s + SE(i) |
| addiu |  001001  | f $d, $s, i  | $d = $s + SE(i) |
|  and  |  100100  | f $d, $s, $t | $d = $s & $t |
|  andi |  001100  | f $d, $s, i  | $t = $s & ZE(i) |
|  div  |  011010  |   f $s, $t   | lo = $s / $t; hi = $s % $t |
|  divu |  011011  |   f $s, $t   | lo = $s / $t; hi = $s % $t |
|  mult |  011000  |   f $s, $t   | hi:lo = $s * $t |
| multu |  011001  |   f $s, $t   | hi:lo = $s * $t |
|  nor  |  100111  | f $d, $s, $t | $d = ~($s \| $t) |
|   or  |  100101  | f $d, $s, $t | $d = $s \| $t |
|  ori  |  001101  | f $d, $s, i  | $t = $s \| ZE(i) |
|  sll  |  000000  | f $d, $t, a  | $d = $t << a |
| sllv  |  000100  | f $d, $t, $s | d = $t << $s |
|  sra  |  000011  | f $d, $t, a  | $d = $t >> a |
|  srav |  000111  | f $d, $t, $s | $d = $t >> $s |
|  srl  |  000010  | f $d, $t, a  | $d = $t >>> a |
|  srlv |  000110  | f $d, $t, $s | $d = $t >>> $s |
|  sub  |  100010  | f $d, $s, $t | $d = $s - $t |
|  subu |  100011  | f $d, $s, $t | $d = $s - $t |
|  xor  |  100110  | f $d, $s, $t | $d = $s ^ $t |
|  xori |  001110  | f $d, $s, i  | $d = $s ^ ZE(i) |

- Comparison Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|  slt   |  101010  | f $d, $s, $t | $d = ($s < $t) |
|  sltu  |  101001  | f $d, $s, $t | $d = ($s < $t) |
|  slti  |  001010  | f $d, $s, i  | $t = ($s < SE(i)) |
|  sltiu |  001001  | f $d, $s, i  | $t = ($s < SE(i)) |

- Branch Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|  beq  |  000100  | o $s, $t, offset | if ($s == $t) pc += i << 2 |
|  bgtz |  000111  |   o $s, offset   | if ($s > 0) pc += i << 2 |
|  blez |  000110  |   o $s, offset   | if ($s <= 0) pc += i << 2 |
|  bne  |  000101  | o $s, $t, offset | if ($s != $t) pc += i << 2 |

- Jump Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|   j   |  000010  |  o index  | pc += i << 2 |
|  jal  |  000011  |  o index  |$31 = pc; pc += i << 2
|  jalr |  001001  |  f labelR | $31 = pc; pc = $s |
|   jr  |  001000  |  f labelR | pc = $s |

- Load Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|   lb   |  100000  |  o $t, i  | ($s) $t = SE (MEM [$s + i]:1) |
|   lbu  |  100100  |  o $t, i  | ($s) $t = ZE (MEM [$s + i]:1) |
|   lh   |  100001  |  o $t, i  | ($s) $t = SE (MEM [$s + i]:2) |
|   lhu  |  100101  |  o $t, i  | ($s) $t = ZE (MEM [$s + i]:2) |
|   lw   |  100011  |  o $t, i  | ($s) $t = MEM [$s + i]:4 |

- Store Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|   sb   |  101000  |  o $t, i  | ($s) MEM [$s + i]:1 = LB ($t) |
|   sh   |  101001  |  o $t, i  | ($s) MEM [$s + i]:2 = LH ($t) |

- Data Movement Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
|  mfhi  |  010000  |  f $d  | $d = hi |
|  mflo  |  010010  |  f $d  | $d = lo |
|  mthi  |  010001  |  f $s  | hi = $s |
|  mtlo  |  010011  |  f $s  | lo = $s |

- Trap Instructions

| Instruction | Opcode/Function | Syntax | Operation |
| :---------: | :-------------: | :----: | :-------: |
| syscall |  001100  |  o   | Cause a System Call exception. |

## Internal representation
The **LMS** will consist of two main components:
- The assembler : That will translate program from assembly to runnable code (machine/byte code)
- The virtual machine (that we can call MIPS CPU) that will run the generated code