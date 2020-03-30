#include <stdio.h>
#include <lmips_opcodes.h>
#include "CuTest.h"
#include "lmips.h"

void testSimulatorInit(CuTest* test) {
    LMips mips;
    initSimulator(&mips, NULL);

    for (int i = 0; i < REG_COUNT; ++i) {
        CuAssertIntEquals(test,  i == $sp ? STACK_SIZE : 0, mips.regs[i]);
    }

    freeSimulator(&mips);
}

void testRunNullProgram(CuTest* test) {
    LMips mips;
    initSimulator(&mips, NULL);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_FAILURE, result);

    freeSimulator(&mips);
}

void testRunInvalidProgram(CuTest* test) {
    LMips mips;
    uint8_t program[] = {
            1, 0, 0, 0
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_FAILURE, result);

    freeSimulator(&mips);
}

void testRunValidProgram(CuTest* test) {
    LMips mips;
    uint8_t program[] = {
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 8, mips.ip);

    freeSimulator(&mips);
}

void testAddProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x50, 0x20, // add $t2, $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 45;
    mips.regs[$t1] = 15;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 60, mips.regs[$t2]);

    freeSimulator(&mips);
}

void testDivProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x10, 0x1A, // div $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 45;
    mips.regs[$t1] = 15;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 3, mips.lo);
    CuAssertIntEquals(test, 0, mips.hi);

    freeSimulator(&mips);
}

void testMfloProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x00, 0x00, 0x50, SPE_MFLO, // mflo $t2
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 15;
    mips.regs[$t1] = 10;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 150, mips.lo);
    CuAssertIntEquals(test, 0, mips.hi);
    CuAssertIntEquals(test, 150, mips.regs[$t2]);

    freeSimulator(&mips);
}

void testSraProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x50, 0x22, // sub $t2, $t0, $t1
            0x00, 0x0A, 0x48, 0x44, // sra $t1, $t2, 1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 1;
    mips.regs[$t1] = 2;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, -1, mips.regs[$t2]);
    CuAssertIntEquals(test, -1 /* -1 << 1 */, mips.regs[$t1]);

    freeSimulator(&mips);
}

void testSllvProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x50, 0x20, // add $t2, $t0, $t1
            0x01, 0x0A, 0x48, 0x04, // sslv $t1, $t2, $t0
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 1;
    mips.regs[$t1] = 2;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 3, mips.regs[$t2]);
    CuAssertIntEquals(test, 6 /* 3 << 1 */, mips.regs[$t1]);

    freeSimulator(&mips);
}

void testSltProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x01, 0x09, 0x50, 0x2A, // slt $t2, $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 1;
    mips.regs[$t1] = 2;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, true, mips.regs[$t2]);

    freeSimulator(&mips);
}

void testJrProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x00, 0x80, 0x00, 0x08, // jr $a0
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL,
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
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

void testJalrProgram(CuTest* test) {
    LMips mips;

    uint8_t program[] = {
            0x00, 0x80, 0x00, 0x09, // jalr $a0
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL,
            0x01, 0x09, 0x10, SPE_MULT, // mult $t0, $t1
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$a0] = 12;
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

CuSuite* getLMipsRTypeInstructionsSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testSimulatorInit);
    SUITE_ADD_TEST(suite, testRunNullProgram);
    SUITE_ADD_TEST(suite, testRunInvalidProgram);
    SUITE_ADD_TEST(suite, testRunValidProgram);
    SUITE_ADD_TEST(suite, testAddProgram);
    SUITE_ADD_TEST(suite, testDivProgram);
    SUITE_ADD_TEST(suite, testMfloProgram);
    SUITE_ADD_TEST(suite, testSraProgram);
    SUITE_ADD_TEST(suite, testSllvProgram);
    SUITE_ADD_TEST(suite, testSltProgram);
    SUITE_ADD_TEST(suite, testJrProgram);
    SUITE_ADD_TEST(suite, testJalrProgram);

    return suite;
}