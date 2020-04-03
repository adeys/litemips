#include <stdio.h>
#include "CuTest.h"

CuSuite* getLMipsRTypeInstructionsSuite();
CuSuite* getLMipsITypeInstructionsSuite();
CuSuite* getLMipsJTypeInstructionsSuite();
CuSuite* getLMipsMemoryInstructionsSuite();

int main(int argc, char const *argv[]) {
    printf("Welcome to Lite MIPS test suite.\n\n");

    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, getLMipsRTypeInstructionsSuite());
    CuSuiteAddSuite(suite, getLMipsITypeInstructionsSuite());
    CuSuiteAddSuite(suite, getLMipsJTypeInstructionsSuite());
    CuSuiteAddSuite(suite, getLMipsMemoryInstructionsSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    return 0;
}
