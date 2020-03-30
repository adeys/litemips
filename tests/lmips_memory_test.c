#include <stdio.h>
#include <lmips_opcodes.h>
#include "CuTest.h"
#include "lmips.h"

void testLoadInvalidMemoryAddressInstruction(CuTest* test) {
    LMips mips;

    // Exceed allowed memory
    uint8_t program[] = {
        0x83, 0x85, 0x00, 0x02, // lb $t0, 2($gp)
        0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
        OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_ERR_MEMORY_ADDR, result);
    CuAssertIntEquals(test, 4, mips.ip);

    freeSimulator(&mips);
}

void testInvalidMemoryOffsetInstruction(CuTest* test) {
    LMips mips;

    // Exceed allowed memory
    uint8_t program[] = {
            0x83, 0xA8, 0x00, 0x02, // lb $t0, 2($sp)
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_ERR_MEMORY_ADDR, result);
    CuAssertIntEquals(test, 4, mips.ip);

    freeSimulator(&mips);
}

void testLbInstruction(CuTest* test) {
    LMips mips;

    // Exceed allowed memory
    uint8_t program[] = {
            0x83, 0xA8, 0x00, 0x00, // lb $t0, ($sp)
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mem_write_byte(mips.memory, mips.regs[$sp], 12);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, 12, mips.regs[$t0]);

    freeSimulator(&mips);
}

void testLhuInstruction(CuTest* test) {
    LMips mips;

    // Exceed allowed memory
    uint8_t program[] = {
        0x97, 0xA8, 0x00, 0x00, // lhu $t0, ($sp)
        0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
        OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mem_write(mips.memory, mips.regs[$sp], -1200);

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test, (uint16_t)(-1200) , mips.regs[$t0]);

    freeSimulator(&mips);
}

void testSbInstruction(CuTest* test) {
    LMips mips;

    // Exceed allowed memory
    uint8_t program[] = {
            0xA3, 0xA8, 0x00, 0x00, // sb $t0, ($sp)
            0x20, 0x02, 0x00, 0x0A, // addi $v0, $zero, 10
            OP_SPECIAL, 0, 0, SPE_SYSCALL
    };

    initSimulator(&mips, program);
    mips.regs[$t0] = 120;

    ExecutionResult result = runSimulator(&mips);
    CuAssertIntEquals(test, EXEC_SUCCESS, result);
    CuAssertIntEquals(test,  mips.regs[$t0], mem_read(mips.memory, mips.regs[$sp]));

    freeSimulator(&mips);
}

CuSuite* getLMipsMemoryInstructionsSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testLoadInvalidMemoryAddressInstruction);
    SUITE_ADD_TEST(suite, testInvalidMemoryOffsetInstruction);
    SUITE_ADD_TEST(suite, testLbInstruction);
    SUITE_ADD_TEST(suite, testLhuInstruction);
    SUITE_ADD_TEST(suite, testSbInstruction);

    return suite;
}