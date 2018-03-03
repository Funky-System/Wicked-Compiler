//
// Created by Bas du Pré on 17-02-18.
//

#include "gtest/gtest.h"
#include "frontend.h"

TEST(Scopes, Basic) {
    auto raw_str = R"wckd(
        import testlib

        do
            var i = 4
            do
                var i = 5
                testlib.assert(i == 5)
            end
            testlib.assert(i == 4)
        end
    )wckd";

    compile_and_run(raw_str);
}

