#include "gtest/gtest.h"
#include "frontend.h"

TEST(Arrays, Indexing) {
auto raw_str = R"wckd(
        import testlib

        var array = [1, 2, 3]

        testlib.assert(array[1] == 2)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Arrays, StoreAndLoad) {
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

TEST(Arrays, Concatenation) {
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

TEST(Arrays, Ranges) {
    auto raw_str = R"wckd(
        import testlib

        var array = 1..5

        testlib.assert(array[1] == 2)
        testlib.assert(array[3] == 4)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Arrays, Multiline) {
    auto raw_str = R"wckd(
        import testlib

        var array = [1, 2,
        3, 8
        , 9, 10
        ]

        testlib.assert(array[1] == 2)
        testlib.assert(array[3] == 8)
        testlib.assert(array[4] == 9)
    )wckd";

    compile_and_run(raw_str);
}

TEST(Arrays, FunctionArrayDereference) {
    auto raw_str = R"wckd(
        import testlib

        function giveArray()
            return ["a", "b", "c"]
        end

        testlib.assert(giveArray()[1] == "b")
    )wckd";

    compile_and_run(raw_str);
}

TEST(Arrays, FunctionArraySlice) {
    auto raw_str = R"wckd(
        import testlib

        function giveArray()
            return ["a", "b", "c"]
        end

        testlib.assert(giveArray()[1:3] == ["b", "c"])
    )wckd";

    compile_and_run(raw_str);
}
