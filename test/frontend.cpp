//
// Created by Bas du Pré on 17-02-18.
//

#include <cstdio>
#include <cerrno>
#include <cstring>
#include "frontend.h"

#include "gtest/gtest.h"

int test_testlib_assert = 0;

extern "C" {

#include "wickedc/wickedc.h"
#include "funkyas/funkyas.h"
#include "funkyvm/funkyvm.h"

void in_vm_assert(CPU_State *state) {
    if (test_testlib_assert == 1) { // used to unit test calling of this function through the testlib module
        test_testlib_assert = 2;
    }

    auto cond_result = STACK_VALUE(state, -1);
    auto error_str = cstr_pointer_from_vm_value(state, STACK_VALUE(state, 0));

    ASSERT_TRUE(cond_result->int_value) << error_str;
}

void load_testlib(CPU_State *state) {
    FILE *fp;
    fp = fopen("testlib.fasm", "r");
    if (fp == nullptr) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not open file testlib.fasm\n");
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    auto numbytes = (size_t) ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    auto code_in = (char *) malloc(numbytes + 1);
    if (fread(code_in, sizeof(char), numbytes, fp) != numbytes) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not read complete file\n");
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    code_in[numbytes] = '\0';

    fclose(fp);

    funky_bytecode_t bc = funky_assemble("testlib.fasm", code_in, 0);
    free(code_in);

    Module module = module_load(state->memory, "testlib", bc);
    free(bc.bytes);
    module_register(state, module);
}

CPU_State compile_and_run(const char *wckd_code) {
    char *fasm = compile_string_to_string("<test:wckd>", wckd_code, 0);
    EXPECT_TRUE(fasm != NULL);
    if (fasm == NULL) return (CPU_State) {0};

    funky_bytecode_t funk = funky_assemble("<test:fasm>", fasm, 0);
    free(fasm);

    EXPECT_TRUE(funk.bytes != NULL);
    if (funk.bytes == NULL) return (CPU_State) {0};
    EXPECT_GT(funk.length, 0);
    if (funk.length == 0) return (CPU_State) {0};

    auto main_memory = (byte_t *) calloc(VM_MEMORY_LIMIT, 1);
    Memory memory;
    memory_init(&memory, main_memory);

    CPU_State state = cpu_init(&memory);

    Module module = module_load(&memory, "<test:funk>", funk);
    free(funk.bytes);
    module_register(&state, module);

    register_syscall(&state, "assert", in_vm_assert);

    cpu_set_entry_to_module(&state, &module);

    load_testlib(&state);

    vm_type_t ret = cpu_run(&state);

    EXPECT_EQ(state.in_error_state, 0) << "VM ended in an error state";

    memory_destroy(&memory);
    free(main_memory);

    return state;
}

};