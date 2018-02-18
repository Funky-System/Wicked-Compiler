#include "gtest/gtest.h"
#include "frontend.h"

TEST(Loops, DoWhile) {
    auto raw_str = R"wckd(
        import testlib

        var i = 0
        do
            testlib.assert(i < 10)
            i += 1
        loop while i < 10

        var test = 0
        i = 11
        do
            test = 1
            i += 1
        loop while i < 10
        testlib.assert(test == 1)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, While) {
    auto raw_str = R"wckd(
        import testlib

        var i = 0
        while i < 10 do
            testlib.assert(i < 10)
            i += 1
        end

        testlib.assert(i == 10)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, For) {
    auto raw_str = R"wckd(
        import testlib

        var j = 0

        for i in 1..5 do
            j += i
        end

        testlib.assert(j == 15)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, BreakAndContinue) {
    auto raw_str = R"wckd(
        import testlib

        var j = 0

        for i in 1..20 do
            if i == 2 then continue
            j += i
            if i == 10 then break
        end

        testlib.assert(j == 53)
    )wckd";

    compile_and_run(raw_str);
}

