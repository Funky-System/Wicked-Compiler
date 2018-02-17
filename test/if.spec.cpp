#include "gtest/gtest.h"
#include "frontend.h"

TEST(If, BasicIfStatement) {
    auto raw_str = R"wckd(
        import testlib

        if 1 == 3 then
            testlib.assert(false)
        end

        if 1 == 1 then
            testlib.assert(true)
        end

    )wckd";

    compile_and_run(raw_str);
}

TEST(If, BasicIfElseStatement) {
    auto raw_str = R"wckd(
        import testlib

        if 1 == 3 then
            testlib.assert(false)
        else
            testlib.assert(true)
        end

        if 3 == 3 then
            testlib.assert(true)
        else
            testlib.assert(false)
        end

    )wckd";

    compile_and_run(raw_str);
}

TEST(If, Oneliner) {
    auto raw_str = R"wckd(
        import testlib

        if 1 == 3 then testlib.assert(false)
        if 3 == 3 then testlib.assert(true)

    )wckd";

    compile_and_run(raw_str);
}

TEST(If, OnelinerElse) {
    auto raw_str = R"wckd(
        import testlib

        if 1 == 3 then testlib.assert(false)
        else testlib.assert(true)

        if 1 == 1 then testlib.assert(true)
        else testlib.assert(false)

        if 1 == 3 then testlib.assert(false) ; else testlib.assert(true)

        if 1 == 1 then testlib.assert(true) ; else testlib.assert(false)

    )wckd";

    compile_and_run(raw_str);
}

TEST(If, ElseIf) {
    auto raw_str = R"wckd(
        import testlib

        if 1 == 3 then testlib.assert(false)
        else if 1 == 1 then testlib.assert(true)
        else testlib.assert(false)

        if 1 == 1 then testlib.assert(true)
        else if 1 == 1 then testlib.assert(false)
        else
            testlib.assert(false)
        end

    )wckd";

    compile_and_run(raw_str);
}

TEST(If, ConditionalExecution) {
    auto raw_str = R"wckd(
        import testlib

        function truthy_should_run()
            testlib.assert(true)
            return true
        end

        function falsy_should_run()
            testlib.assert(true)
            return false
        end

        function truthy_should_NOT_run()
            testlib.assert(false)
            return true
        end

        function falsy_should_NOT_run()
            testlib.assert(false)
            return false
        end

        truthy_should_run() && truthy_should_run()
        truthy_should_run() || truthy_should_NOT_run()
        falsy_should_run() && falsy_should_NOT_run()
        falsy_should_run() || falsy_should_run()

        truthy_should_run() && truthy_should_run() || falsy_should_NOT_run()
        truthy_should_run() || truthy_should_NOT_run() || truthy_should_NOT_run()
        falsy_should_run() || truthy_should_run() || truthy_should_NOT_run()
        falsy_should_run() && falsy_should_NOT_run() || falsy_should_run()

    )wckd";

    compile_and_run(raw_str);
}
