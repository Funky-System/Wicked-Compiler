#include "gtest/gtest.h"
#include "frontend.h"

TEST(Strings, Comparison) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert("abc" == "abc")
        testlib.assert("abc" != "cba")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Strings, Concatenations) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert("a" + "b" == "ab")
        testlib.assert("a" + "b" + "c" == "abc")
        testlib.assert(1 + "a" == "1a")
        testlib.assert("a" + 1 == "a1")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Strings, Slice) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert("abcd"[0:2] == "ab")
        testlib.assert("abcd"[2:3] == "c")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Strings, FunctionStringSlice) {
    auto raw_str = R"wckd(
        import testlib

        function giveString()
            return "abc"
        end

        testlib.assert(giveString()[1:3] == "bc")
    )wckd";

    compile_and_run(raw_str);
}
