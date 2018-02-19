#include "gtest/gtest.h"
#include "frontend.h"

TEST(Maps, Basics) {
    auto raw_str = R"wckd(
        import testlib

        var map = { "a": 1, "b": 2 }

        testlib.assert(map["a"] == 1)
        testlib.assert(map["A"]== empty)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Maps, Multiline) {
    auto raw_str = R"wckd(
        import testlib

        var map = {
            "a": 1,
            "b": 2,

            "c": 3
        }

        testlib.assert(map["a"] == 1)
        testlib.assert(map["A"]== empty)
        testlib.assert(map["c"]== 3)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Maps, Empty) {
    auto raw_str = R"wckd(
        import testlib

        var map = {

        }

        var map2 = {}

        testlib.assert(map["A"]== empty)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Maps, BasicStore) {
    auto raw_str = R"wckd(
        import testlib

        var map = { "a": 1, "b": 2 }

        testlib.assert(map["a"] == 1)

        map["a"] = 5
        testlib.assert(map["a"] == 5)

        testlib.assert(map["A"]== empty)
        map["A"] = 44
        testlib.assert(map["A"]== 44)

        map["xxx"] = 23
        testlib.assert(map["xxx"]== 23)

        map["xx" + "x"] = 90
        testlib.assert(map["x" + "x" + "x"]== 90)
    )wckd";

    compile_and_run(raw_str);
}

