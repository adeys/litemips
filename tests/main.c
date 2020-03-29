#include <stdio.h>
#include "CuTest.h"

CuSuite* getLMipsSimulatorSuite();

int main(int argc, char const *argv[]) {
    printf("Welcome to Lite VM test suite.\n\n");

    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, getLMipsSimulatorSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    return 0;
}
