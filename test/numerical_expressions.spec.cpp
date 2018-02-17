#include "gtest/gtest.h"
#include "frontend.h"

TEST(NumericalExpressions, SimpleOperators) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert(1 + 2 == 3)
    )wckd";

    compile_and_run(raw_str);
}
