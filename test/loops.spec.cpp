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

    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, While) {
    auto raw_str = R"wckd(
        import testlib

        var array = [1, 2, 3]

        testlib.assert(array[1] == 2)
        array[1] = 4
        testlib.assert(array[1] == 4)

        array[5] = 9
        testlib.assert(array[5] == 9)
        testlib.assert(array[4] == empty)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, For) {
    auto raw_str = R"wckd(
        import testlib

        var array1 = [1, 2, 3]
        var array2 = ["a", "b", "c"]

        testlib.assert(array1[1] == 2)
        testlib.assert(array2[1] == "b")

        var array3 = array1 + array2

        testlib.assert(array3[2] == 3)
        testlib.assert(array3[3] == "a")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Loops, BreakAndContinue) {
    auto raw_str = R"wckd(
        import testlib

        var array1 = [1, 2, 3]
        var array2 = ["a", "b", "c"]

        testlib.assert(array1[1] == 2)
        testlib.assert(array2[1] == "b")

        var array3 = array1 + array2

        testlib.assert(array3[2] == 3)
        testlib.assert(array3[3] == "a")
    )wckd";

    compile_and_run(raw_str);
}

