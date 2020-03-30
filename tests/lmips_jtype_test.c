#include <stdio.h>
#include <lmips_opcodes.h>
#include "CuTest.h"
#include "lmips.h"

void testJProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x08, 0x00, 0x00, 0x03, // j 12
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL,
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$a0] = 12;
    mips.regs[$t0] = 15;
    mips.regs[$t1] = 10;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 24, mips.ip);
    CuAssertIntEquals(test, 150, mips.lo);
    CuAssertIntEquals(test, 0, mips.hi);

    freeSimulator(&mips);
}

void testJalProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x0C, 0x00, 0x00, 0x03, // jal 12
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL,
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 15;
    mips.regs[$t1] = 10;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 24, mips.ip);
    CuAssertIntEquals(test, 4, mips.regs[$ra]);
    CuAssertIntEquals(test, 150, mips.lo);
    CuAssertIntEquals(test, 0, mips.hi);

    freeSimulator(&mips);
}

CuSuite* getLMipsJTypeInstructionsSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testJProgram);
    SUITE_ADD_TEST(suite, testJalProgram);

    return suite;
}