//
// Created by Bas du Pré on 16-02-18.
//

#include "gtest/gtest.h"

extern "C" {

#include "wickedc/wickedc.h"
#include "funkyas/funkyas.h"

int main(int argc, char **argv) {
    char *fasm = compile_string_to_string("<test:wckd>", "import io\nio.print(\"test\\n\")\n", 0);

    size_t size_out;
    char *funk = assemble_string_to_string("<test:fasm>", fasm, 0, &size_out);
    free(fasm);



    free(funk);

    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    return ret;
}

};