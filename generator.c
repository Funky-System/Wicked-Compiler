//
// Created by Bas du Pré on 23-01-18.
//

#include "generator.h"
#include <assert.h>

void append_output(generator_state_t *state, const char *format, ...) {
    if (state->fp == NULL) {
        return;
    }
    va_list args;
    va_start(args, format);
    vfprintf(state->fp, format, args);
    va_end(args);
}

int generate(const char *filename, const char *filename_output, mpc_ast_t *ast) {
    generator_state_t state = { 0 };
    state.filename = filename;

    if (filename_output != NULL) {
        state.fp = fopen(filename_output, "wb");

        if (state.fp == NULL) {
            int errnum = errno;
            printf("Error: Could not write to file %s\n", filename_output);
            printf("%s\n", strerror(errnum));
            exit(EXIT_FAILURE);
        }
    } else {
        state.fp = NULL;
    }

    hashmap_init(&state.symbol_table, hashmap_hash_string, hashmap_compare_string, 0);

    // the root is always called '>'
    assert('>' == ast->tag[0]);
    // the first child is a regex for '^'
    assert(0 == strcmp("regex", ast->children[0]->tag));

    // the last child is a regex for '$'
    assert(0 == strcmp("regex", ast->children[ast->children_num - 1]->tag));

    int num_globals = 0;
    reserve_globals(&state, ast, -1, &num_globals);

    for (int i = 1; i < (ast->children_num - 1); i++) {
        mpc_ast_t *part = ast->children[i];

        if (strcmp("function|>", part->tag) == 0) generate_function(&state, part);
        if (strcmp("class|>", part->tag) == 0) generate_class(&state, part);
        if (strcmp("stmt|>", part->tag) == 0) generate_stmt(&state, part);
    }

    //append_output(&state, "ld.uint 0\nst.reg %%rr\n");

    hashmap_destroy(&state.symbol_table);

    if (state.fp != NULL) fclose(state.fp);

    return 0;
}
