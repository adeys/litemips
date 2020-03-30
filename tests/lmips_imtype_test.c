#include <stdio.h>
#include <lmips_opcodes.h>
#include "CuTest.h"
#include "lmips.h"

void testAddiInstructionWithOverflow(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x21, 0x09, 0x00, 0x64, // addi $t1, $t0, 100
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = INT32_MAX;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_ERR_INT_OVERFLOW, result);
    CuAssertIntEquals(test, 0, mips.regs[$t1]);

    freeSimulator(&mips);
}

void testAddiuInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x24, 0x08, 0x00, 0x64, // addiu $t0, $0, 100
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
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
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
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
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, true, mips.regs[$t0]);

    freeSimulator(&mips);
}

void testBeqInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x10, 0x80, 0x00, 0x01, // beq $0, $a0, 4
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$a0] = 0;
    mips.regs[$t0] = 15;
    mips.regs[$t1] = 10;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 16, mips.ip);
    CuAssertIntEquals(test, 0, mips.lo);

    freeSimulator(&mips);
}

void testBlezInstruction(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
        0x18, 0x80, 0x00, 0x02, // blez $a0, 8
        0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
        OP_SPECIAL, 0, 0, SPE_SYSCALL,
        0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
        0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
        OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$a0] = -5;
    mips.regs[$t0] = 15;
    mips.regs[$t1] = 10;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 24, mips.ip);
    CuAssertIntEquals(test, 150, mips.lo);
    CuAssertIntEquals(test, 0, mips.hi);

    freeSimulator(&mips);
}

CuSuite* getLMipsITypeInstructionsSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testAddiInstructionWithOverflow);
    SUITE_ADD_TEST(suite, testAddiuInstruction);
    SUITE_ADD_TEST(suite, testSltiInstruction);
    SUITE_ADD_TEST(suite, testSltiuInstruction);
    SUITE_ADD_TEST(suite, testBeqInstruction);
    SUITE_ADD_TEST(suite, testBlezInstruction);

    return suite;
}