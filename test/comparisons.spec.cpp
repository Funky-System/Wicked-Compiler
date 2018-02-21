#include "gtest/gtest.h"
#include "frontend.h"

TEST(Comparisons, SimpleComparisons) {
auto raw_str = R"wckd(
        import testlib

        testlib.assert(1 != 3)
        testlib.assert(1 != "1")
        testlib.assert(4 == 4)
        testlib.assert(5 >= 4)
        testlib.assert(4 >= 4)
        testlib.assert(4 <= 4)
        testlib.assert(3 <= 4)
        testlib.assert(3 < 4)
        testlib.assert(5 > 4)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Comparisons, Combinations) {
    auto raw_str = R"wckd(
        import testlib
        testlib.assert(4 == 3 || 4 == 4)
        testlib.assert("4" == "3" || "4" == "4")
        testlib.assert("4" == "4" && "4" == "4")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Comparisons, Call) {
    auto raw_str = R"wckd(
        import testlib

        function fun(a)
            return a + 1
        end

        function b(s)
            testlib.assert(fun(s) == "a2" || fun(s) == "a1")
        end
        b("a")

        function c(s)
            testlib.assert(fun(s) == "a1" || fun(s) == "b2")
        end
        c("a")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Comparisons, CallAnd) {
    auto raw_str = R"wckd(
        import testlib

        function fun(a)
            return a + 1
        end

        function b(s)
            testlib.assert(fun(s) == "a1" && fun(s) == "a1")
        end
        b("a")

        function c(s)
            testlib.assert(fun(s) == "a1" && !(fun(s) == "a2"))
        end
        c("a")
    )wckd";

    compile_and_run(raw_str);
}
