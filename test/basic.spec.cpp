//
// Created by Bas du Pré on 17-02-18.
//

#include "gtest/gtest.h"
#include "frontend.h"

TEST(Basic, CanCalculate) {
    auto raw_str = R"wckd(
        var a = 1 + 3
        asm "ld.deref @global_0
             st.reg %r0"
    )wckd";

    auto end_state = compile_and_run(raw_str);
    EXPECT_EQ(end_state.running, 0);
    EXPECT_EQ(end_state.r0.type, VM_TYPE_INT);
    EXPECT_EQ(end_state.r0.int_value, 4);
}

TEST(Basic, CanCallFunction) {
    auto raw_str = R"wckd(
        function b(n)
            return n + 4
        end
        var a = b(2) + 3
        asm "ld.deref @global_0
             st.reg %r0"
    )wckd";

    auto end_state = compile_and_run(raw_str);
    EXPECT_EQ(end_state.running, 0);
    EXPECT_EQ(end_state.r0.type, VM_TYPE_INT);
    EXPECT_EQ(end_state.r0.int_value, 9);
}

TEST(Basic, TestlibWorking) {
    auto raw_str = R"wckd(
        import testlib

        testlib.assert(2 == 2)
    )wckd";

    test_testlib_assert = 1;
    compile_and_run(raw_str);
    EXPECT_EQ(test_testlib_assert, 2) << "The testlib.assert syscall wasn't called";
    test_testlib_assert = 0;
}
