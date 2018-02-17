#include "gtest/gtest.h"
#include "frontend.h"

TEST(NumericalExpressions, SimpleOperators) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert(1 + 2 == 3, "+")
        testlib.assert(4 - 3 == 1, "-")
        testlib.assert(4 * 3 == 12, "*")
        testlib.assert(10 / 5 == 2, "/")
        testlib.assert(1 / 4.0 == 0.25, "/")
        testlib.assert(9 % 6 == 3, "%")
        testlib.assert(3 ** 5 == 243, "**")
        testlib.assert(4 - 13 == -9, "- (negative)")
        testlib.assert(-3 * -1 == 3, "* (negative)")
    )wckd";

    compile_and_run(raw_str);
}

TEST(NumericalExpressions, OperatorPrecedence) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert(1 + 2 * 5 == 11)
        testlib.assert((1 + 2) * 5 == 15)
        testlib.assert(4 - 3 / 2.0 == 2.5)
        testlib.assert((4 - 3) / 2.0 == 0.5)
        testlib.assert(4 * 3 + 3 == 15)
        testlib.assert(4 * (3 + 3) == 24)
        testlib.assert(18 / 6 * 3 == 9)
        testlib.assert(3 * 3 ** 3 == 81)
        testlib.assert(3 ** 4 ** 2 == 6561)
        testlib.assert(4 - 13  == -9)
        testlib.assert(-3 * -1 == 3)
    )wckd";

    compile_and_run(raw_str);
}
