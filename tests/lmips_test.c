#include <stdlib.h>
#include "CuTest.h"
#include "lmips.h"

void testSimulatorInit(CuTest* test) {
    LMips mips;
    initSimulator(&mips);

    for (int i = 0; i < REG_COUNT; ++i) {
        CuAssertIntEquals(test, 0, mips.regs[i]);
    }

    freeSimulator(&mips);
}

CuSuite* getLMipsSimulatorSuite() {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testSimulatorInit);

    return suite;
}