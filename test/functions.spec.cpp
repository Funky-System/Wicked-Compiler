#include "gtest/gtest.h"
#include "frontend.h"

TEST(Functions, Params) {
    auto raw_str = R"wckd(
        import testlib

        function fun(a, b)
            return a - b
        end

        testlib.assert(fun(4, 1) == 3)
        testlib.assert(fun(6.4, 2.2) == 4.2)
        testlib.assert(fun(-1, 4) == -5)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Functions, Recursion) {
    auto raw_str = R"wckd(
        import testlib

        function fun(val, ctr)
            if (ctr >= 5) then return val + val
            return fun(val + val, ctr + 1)
        end

        testlib.assert(fun(1, 0) == 64)
        testlib.assert(fun(1, 1) == 32)
        testlib.assert(fun(4.1, 0) == 262.4)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Functions, DefaultParams) {
    auto raw_str = R"wckd(
        import testlib

        function fun(a = 1, b = 2)
            return a + b
        end

        testlib.assert(fun(4, 1) == 5)
        testlib.assert(fun() == 3)
        testlib.assert(fun(5) == 7)
        testlib.assert(fun(empty, 9) == 10)
    )wckd";

    compile_and_run(raw_str);
}
