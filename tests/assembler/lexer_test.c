#include <stdio.h>
#include "CuTest.h"
#include "lexer.h"

void testTokenizeEmptyString(CuTest* test) {
    Lexer lexer;
    initLexer(&lexer);

    tokenize(&lexer, "");
    // On empty string we have only EOF Token
    CuAssertIntEquals(test, 1, lexer.tokens.count);
    CuAssertIntEquals(test, STATE_OK, lexer.state);
    CuAssertIntEquals(test, T_EOF, lexer.tokens.tokens[0].type);

    freeLexer(&lexer);
}

void testTokenizeSingleLine(CuTest* test) {
    Lexer lexer;
    initLexer(&lexer);

    const char* program = "hello_msg: .asciiz \"Hello World\\n\"";
    tokenize(&lexer, program);

    CuAssertIntEquals(test, 4, lexer.tokens.count);
    CuAssertIntEquals(test, STATE_OK, lexer.state);
    CuAssertIntEquals(test, T_LABEL, lexer.tokens.tokens[0].type);
    CuAssertIntEquals(test, T_DIRECTIVE, lexer.tokens.tokens[1].type);
    CuAssertIntEquals(test, T_STRING, lexer.tokens.tokens[2].type);
    CuAssertIntEquals(test, T_EOF, lexer.tokens.tokens[3].type);
    CuAssertIntEquals(test, 1, lexer.line);

    freeLexer(&lexer);
}

void testTokenizeNumber(CuTest* test) {
    Lexer lexer;
    initLexer(&lexer);

    const char* program = "0x20 125 012";
    tokenize(&lexer, program);

    CuAssertIntEquals(test, 4, lexer.tokens.count);
    CuAssertIntEquals(test, STATE_OK, lexer.state);

    // Check parsed token value
    CuAssertIntEquals(test, T_SCALAR, lexer.tokens.tokens[0].type);
    CuAssertIntEquals(test, 32, lexer.tokens.tokens[0].value.scalar);

    CuAssertIntEquals(test, T_SCALAR, lexer.tokens.tokens[1].type);
    CuAssertIntEquals(test, 125, lexer.tokens.tokens[1].value.scalar);

    CuAssertIntEquals(test, T_SCALAR, lexer.tokens.tokens[2].type);
    CuAssertIntEquals(test, 10, lexer.tokens.tokens[2].value.scalar);

    CuAssertIntEquals(test, T_EOF, lexer.tokens.tokens[3].type);
    CuAssertIntEquals(test, 1, lexer.line);

    freeLexer(&lexer);
}

void testTokenizeValidProgram(CuTest* test) {
    Lexer lexer;
    initLexer(&lexer);

    const char* program = "# Daniel J. Ellard -- 02/21/94\n"
                          "# hello.asm-- A \"Hello World\" program.\n"
                          "# Registers used:\n"
                          "# $v0 - syscall parameter and return value.\n"
                          "# $a0 - syscall parameter-- the string to print.\n"
                          ".text\n"
                          "main:\n"
                          "la $a0, hello_msg # load the addr of hello_msg into $a0.\n"
                          "li $v0, 4 # 4 is the print_string syscall.\n"
                          "syscall # do the syscall.\n"
                          "li $v0, 10 # 10 is the exit syscall.\n"
                          "syscall # do the syscall.\n"
                          "# Data for the program:\n"
                          ".data\n"
                          "hello_msg: .asciiz \"Hello World\\n\"\n"
                          "# end hello.asm";
    tokenize(&lexer, program);

    CuAssertIntEquals(test, STATE_OK, lexer.state);
    CuAssertIntEquals(test, 21, lexer.tokens.count);
    CuAssertIntEquals(test, 16, lexer.line);

    freeLexer(&lexer);
}

void testTokenizeInvalidProgram(CuTest* test) {
    Lexer lexer;
    initLexer(&lexer);

    const char* program = "# If $t0 > $t1, branch to t0_bigger,\n"
                          "bgt $t0, $t1, t0_bigger\n"
                          "move $t2, $t1 # otherwise, copy $t1 into $t2.\n"
                          "b endif # and then branch to endif\n"
                          "t0_bigger:\n"
                          "move $t2, $t0 # copy $t0 into $t2\n"
                          "endif: 0x12 >= 15";
    tokenize(&lexer, program);

    CuAssertIntEquals(test, STATE_ERROR, lexer.state);

    freeLexer(&lexer);
}

CuSuite* getLexerTestSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testTokenizeEmptyString);
    SUITE_ADD_TEST(suite, testTokenizeSingleLine);
    SUITE_ADD_TEST(suite, testTokenizeNumber);
    SUITE_ADD_TEST(suite, testTokenizeValidProgram);
    SUITE_ADD_TEST(suite, testTokenizeInvalidProgram);

    return suite;
}