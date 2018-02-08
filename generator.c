//
// Created by Bas du Pré on 23-01-18.
//

#include "generator.h"
#include <assert.h>
#include <mpc/mpc.h>

void append_output(generator_state_t *state, const char *format, ...) {
    char buffer[strlen(format) + 2048];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    state->output = realloc(state->output, strlen(state->output) + strlen(buffer) + 1);
    strcat(state->output, buffer);
}

void append_debug_setcontext(generator_state_t *state, mpc_ast_t *ast) {
    if (!state->debug) return;

    append_output(state, "debug.setcontext @filename, %d, %d\n", ast->state.row + 1, ast->state.col);
}

void append_debug_enterscope(generator_state_t *state, const char* prefix, const char* name) {
    if (!state->debug) return;

    append_output(state, "debug.enterscope \"%s%s\"\n", prefix, name);
}

void append_debug_leavescope(generator_state_t *state) {
    if (!state->debug) return;

    append_output(state, "debug.leavescope\n");
}

char* generate(const char *filename_hint, int debug, mpc_ast_t *ast) {
    generator_state_t state = { 0 };
    state.filename = filename_hint;

    state.output = malloc(0);

    state.debug = debug;

    append_output(&state, "section .data\n@filename: data \"%s\"\nsection .text\n\n", filename_hint);

    hashmap_init(&state.symbol_table, hashmap_hash_string, hashmap_compare_string, 0);

    // the root is always called '>'
    assert('>' == ast->tag[0]);
    // the first child is a regex for '^'
    assert(0 == strcmp("regex", ast->children[0]->tag));

    // the last child is a regex for '$'
    assert(0 == strcmp("regex", ast->children[ast->children_num - 1]->tag));

    int num_globals = 0, num_params = 0;
    populate_symbol_table(&state, ast, -1, &num_globals, &num_params, SYMBOL_TYPE_GLOBAL);
    //printf("symbol table after globals:\n");
    //print_symbol_table(&state);

    for (int i = 1; i < (ast->children_num - 1); i++) {
        mpc_ast_t *part = ast->children[i];

        if (strcmp("function|>", part->tag) == 0) generate_function(&state, part, "");
        if (strcmp("class|>", part->tag) == 0) generate_class(&state, part);
        if (strcmp("stmt|>", part->tag) == 0) generate_stmt(&state, part);
    }

    //append_output(&state, "ld.uint 0\nst.reg %%rr\n");

    hashmap_destroy(&state.symbol_table);

    return state.output;
}
