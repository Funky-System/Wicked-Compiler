#include "gtest/gtest.h"
#include "frontend.h"

TEST(Classes, Basic) {
    auto raw_str = R"wckd(
        import testlib

        class A
            var num = 0
            function inc_num(by)
                this.num += by
            end
        end

        var a = new A
        var a2 = new A

        testlib.assert(a.num == 0)
        a.inc_num(1)
        testlib.assert(a.num == 1)
        a.inc_num(2)
        testlib.assert(a.num == 3)
        testlib.assert(a2.num == 0)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Classes, This) {
    auto raw_str = R"wckd(
        import testlib

        class A
            var num = 0
            function inc_num(by)
                this.set_num(this.num + by)
            end

            function set_num(to)
                this.num = to
            end
        end

        var a = new A
        var a2 = new A

        testlib.assert(a.num == 0)
        a.inc_num(1)
        testlib.assert(a.num == 1)
        a.inc_num(2)
        testlib.assert(a.num == 3)
        testlib.assert(a2.num == 0)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Classes, Constructor) {
    auto raw_str = R"wckd(
        import testlib

        class A
            function new(num_val = -4)
                this.set_num(num_val + 4)
            end

            var num = 0
            function inc_num(by)
                this.set_num(this.num + by)
            end

            function set_num(to)
                this.num = to
            end
        end

        var a = new A(1)
        var a2 = new A

        testlib.assert(a.num == 5)
        a.inc_num(1)
        testlib.assert(a.num == 6)
        a.inc_num(2)
        testlib.assert(a.num == 8)
        testlib.assert(a2.num == 0)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Classes, Inheritance) {
    auto raw_str = R"wckd(
        import testlib

        class B
            var b_var = 50
            function new()
                this.b_var += 4
            end

            function b_func(a)
                return a + 1
            end
        end

        class A extends B
            var num = 0
            function inc_num(by)
                this.num += this.b_func(by)
            end

        end

        var a = new A

        testlib.assert(a.num == 0)
        testlib.assert(a.b_var == 54)
        a.inc_num(1)
        testlib.assert(a.num == 2)
        a.inc_num(2)
        testlib.assert(a.num == 5)
        testlib.assert(a.b_func(1) == 2)


    )wckd";

    compile_and_run(raw_str);
}

// super keyword
TEST(Classes, SuperKeyword) {
    auto raw_str = R"wckd(
        import testlib

        class B
            var b_var = 50
            function new()
                this.b_var += 4
            end

            function b_func(a)
                return a + 1
            end
        end

        class A extends B
            function new()

            end
        end

        class A2 extends B
            function new()
                super.new()
            end

            function b_func(a)
                return super.b_func(a + 1)
            end
        end

        var a = new A
        testlib.assert(a.b_var == 50)
        var a2 = new A2
        testlib.assert(a2.b_var == 54)
        testlib.assert(a2.b_func(1) == 3)

    )wckd";

    compile_and_run(raw_str);
}

TEST(Classes, Prototyping) {
    auto raw_str = R"wckd(
        import testlib

        class B

        end

        class A extends B

        end

        var a1 = new A
        var a2 = new A
        (prototypeof A).test_var = 123
        testlib.assert(a1.test_var == 123)
        testlib.assert(a2.test_var == 123)
        var a3 = new A
        testlib.assert(a3.test_var == 123)

        var b1 = new B
        testlib.assert(b1.test_var == empty)
    )wckd";

    compile_and_run(raw_str);
}

//TEST(Classes, ToString) {
//    auto raw_str = R"wckd(
//        import testlib
//
//        class B
//            var b_var = 1
//            function string()
//                return "<" + this.b_var + ">"
//            end
//        end
//
//        class A extends B
//            var a_var = 1
//            function string()
//                return super + "<" + this.a_var + ">"
//            end
//        end
//
//        var b = new B
//        b.b_var = 13
//        testlib.assert(b.string() == "<13>")
//        testlib.assert(b.string() == "<13>")
//        var a = new A
//        a.b_var = 14
//        a.a_var = 11
//        testlib.assert(a.string() == "<14><11>")
//
//    )wckd";
//
//    compile_and_run(raw_str);
//}
