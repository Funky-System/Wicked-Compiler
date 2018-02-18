#include "gtest/gtest.h"
#include "frontend.h"

TEST(Enums, Enums) {
auto raw_str = R"wckd(
        import testlib

        enum TestEnum
            Var1, var, var2 = 30, Hoi,
            bla,
            test = 1
        end

        testlib.assert(TestEnum.Var1 == 0)
        testlib.assert(TestEnum.var == 1)
        testlib.assert(TestEnum.var2 == 30)
        testlib.assert(TestEnum.Hoi == 31)
        testlib.assert(TestEnum.bla == 32)
        testlib.assert(TestEnum.test == 1)
    )wckd";

compile_and_run(raw_str);
}

