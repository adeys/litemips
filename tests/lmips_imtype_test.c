#include <stdio.h>
#include <lmips_opcodes.h>
#include "CuTest.h"
#include "lmips.h"

void testAddiInstructionWithOverflow(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x21, 0x09, 0x00, 0x64, // addi $t1, $t0, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = INT32_MAX;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_EXCP_INT_OVERFLOW, result);
    CuAssertIntEquals(test, 0, mips.regs[$t1]);

    freeSimulator(&mips);
}

void testAddiuInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x24, 0x08, 0x00, 0x64, // addiu $t0, $0, 100
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 100, mips.regs[$t0]);

    freeSimulator(&mips);
}

void testSltiInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x28, 0x08, 0xFF, 0x9C, // slti $t0, $0, -100
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, false, mips.regs[$t0]);

    freeSimulator(&mips);
}

void testSltiuInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            // Here the 16-bit number will be considered unsigned
            0x2C, 0x08, 0xFF, 0x9C, // slti $t0, $0, 65436
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, true, mips.regs[$t0]);

    freeSimulator(&mips);
}

CuSuite* getLMipsITypeInstructionsSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testAddiInstructionWithOverflow);
    SUITE_ADD_TEST(suite, testAddiuInstruction);
    SUITE_ADD_TEST(suite, testSltiInstruction);
    SUITE_ADD_TEST(suite, testSltiuInstruction);

    return suite;
}