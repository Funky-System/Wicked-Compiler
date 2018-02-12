#include <assert.h>
#include "generator.h"

void generate_arrayInit(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("arrayInit|>", ast->tag));
    int num = 0;
    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "exp|>") != 0) {
            continue;
        }
        num++;
        generate_exp(state, ast->children[i]);
    }

    append_output(state, "ld.arr %d\n", num);
}

void generate_arrIndex(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("arrIndex|>", ast->tag));

    if (state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        //append_output(state, "dup\n");
        if (strcmp(ast->children[1]->tag, "exp|>") == 0 && strcmp(ast->children[2]->contents, "]") == 0) {
            generate_exp(state, ast->children[1]);
        } else if (strcmp(ast->children[1]->contents, "]") == 0) {
            // this is an array dereference to something that does not exist yet
            append_output(state, "dup\narr.len\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: unexpected array indexer\n", state->filename,
                    ast->children[1]->state.row + 1, ast->children[1]->state.col);
            exit(EXIT_FAILURE);
        }

        append_output(state, "st.arrelem\n");
    } else {
        if (strcmp(ast->children[1]->tag, "exp|>") == 0) {
            generate_exp(state, ast->children[1]);
            if (ast->children_num > 2 && strcmp(ast->children[2]->contents, ":") == 0) {
                // slice
                generate_exp(state, ast->children[3]);
                append_output(state, "arr.slice\n");
            } else {
                append_output(state, "ld.arrelem\n");
            }
        } else if (strcmp(ast->children[1]->contents, "]") == 0) {
            // this is an array dereference to something that does not exist yet
            //printf("dup\narr.len\n");
            fprintf(stderr, "%s:%ld:%ld error: empty array index\n", state->filename, ast->children[1]->state.row + 1,
                    ast->children[1]->state.col);
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "%s:%ld:%ld error: unexpected array indexer\n", state->filename,
                    ast->children[1]->state.row + 1, ast->children[1]->state.col);
            exit(EXIT_FAILURE);
        }

    }
}