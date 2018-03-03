#include <assert.h>
#include "generator.h"

void generate_default_params(generator_state_t *state, mpc_ast_t *ast);

void generate_syscall(generator_state_t *state, mpc_ast_t *ast, const char* prefix) {
    assert(0 == strcmp("function|>", ast->tag));
    assert(strcmp(ast->children[0]->contents, "syscall") == 0);

    char *name, *alias;
    if (strcmp(ast->children[1]->tag, "syscallAlias|>") == 0) {
        alias = ast->children[1]->children[1]->contents;
        name = ast->children[2]->contents;
    } else {
        name = ast->children[1]->contents;
        alias = name;
    }
    append_output(state,"# Syscall name: %s%s\n", prefix, name);
    append_output(state,"jmp @%s%s__end\n", prefix, name);
    append_output(state,"%s%s: \n", prefix, name);

    append_debug_enterscope(state, prefix, name);
    enter_scope(state, name, NULL, NULL);

    mpc_ast_t *args = ast->children[3];
    if (strcmp(args->tag, "args|>") != 0) {
        args = ast->children[4];
    }

    int num_params = 0;
    for (int i = 0; i < args->children_num; i++) {
        if (strcmp(args->children[i]->tag, "arg|>") == 0) {
            num_params++;
        }
    }

    append_output(state,"args.accept %d\n", num_params);

    if (num_params > 0) {
        generate_default_params(state, args);
    }

    for (int i = 0; i < num_params; i++) {
        append_output(state, "ld.arg %d\n", i);
    }
    append_output(state,"syscall.byname \"%s\"\n", alias);
    append_output(state, "ajs -%d\n", num_params);

    append_output(state,"args.cleanup\n");
    append_debug_leavescope(state);
    append_output(state,"ret\n");
    append_output(state,"@%s%s__end: ", prefix, name);
    append_output(state, "\n");
    leave_scope(state, 1);
}


void generate_function(generator_state_t *state, mpc_ast_t *ast, const char* prefix) {
    assert(0 == strcmp("function|>", ast->tag));

    if (strcmp(ast->children[0]->contents, "syscall") == 0) {
        generate_syscall(state, ast, prefix);
        return;
    }

    char *name = ast->children[1]->contents;

    if (strcmp(name, "string") == 0) {
        state->is_conv_method = 1;
    }

    append_output(state,"# Function name: %s%s\n", prefix, name);
    append_output(state,"jmp @%s%s__end\n", prefix, name);
    append_output(state,"%s%s: \n", prefix, name);

    append_debug_enterscope(state, prefix, name);

    //char *prefix_plus_name = malloc(strlen(prefix) + strlen(name) + 1);
    //strcpy(prefix_plus_name, prefix); strcat(prefix_plus_name, name);
    enter_scope(state, name, NULL, NULL);

    // scan for locals
    int num_locals = 1 /* 'this' is the first local */, num_params = 0;

    if (state->is_conv_method) {
        num_locals++; // local for the relative stack pos of the to be return result
    }

    populate_symbol_table(state, ast, "", &num_locals, &num_params, SYMBOL_TYPE_LOCAL);
    //printf("symbol table after locals for function %s%s:\n", prefix, name);
    //print_symbol_table(state);
    append_output(state,"args.accept %d\n", num_params);
    append_output(state,"locals.res %d\n", num_locals);
    append_output(state, "ld.reg %%r1\n");
    append_output(state, "st.local 0\n");
    if (state->is_conv_method) {
        append_output(state, "ld.reg %%r7\n");
        append_output(state, "st.local 1\n");
    }

    if (num_params > 0) {
        if (state->is_conv_method) {
            fprintf(stderr, "%s:%ld:%ld error: conversion method do not take any parameters\n", state->filename,
                    ast->children[3]->state.row + 1, ast->children[3]->state.col);
            exit(EXIT_FAILURE);
        } else {
            generate_default_params(state, ast->children[3]);
        }
    }

    int index = mpc_ast_get_index(ast, "stmt|>");
    if (index == -1) index = ast->children_num - 2;

    for (int i = index; i < ast->children_num; i++) {
        if (strcmp("stmt|>", ast->children[i]->tag) == 0) {
            generate_stmt(state, ast->children[i]);
        } else if (strcmp("string", ast->children[i]->tag) == 0 && strcmp("end", ast->children[i]->contents) == 0) {
            if (state->is_conv_method) {
                append_output(state, "ld.empty\n");
                append_output(state, "ld.local 1\n");
                append_output(state, "ld.int 1\nsub\n");
                append_output(state, "st.arg.pop\n");
            }

            append_output(state,"locals.cleanup\nargs.cleanup\nld.int 0\nst.reg %%rr\n");
            //append_debug_leavescope(state);
            append_output(state,"ret\n");
            append_output(state,"@%s%s__end: ", prefix, name);
            append_output(state,"# end\n");
        }
    }

    append_output(state, "\n");
    leave_scope(state, 1);
    state->is_conv_method = 0;
}

void generate_default_params(generator_state_t *state, mpc_ast_t *ast) {
    assert(strcmp(ast->tag, "args|>") == 0);

    for (int i = 0, argnum = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "arg|>") == 0) {
            mpc_ast_t *arg = ast->children[i];
            if (arg->children_num > 1 && strcmp(arg->children[1]->contents, "=") == 0) {
                // this arg has a default value
                int skipId = state->uniqueid++;
                append_output(state, "# default argument value for %d\n", argnum);
                append_output(state, "ld.arg %d\n", argnum);
                append_output(state, "is.empty\n");
                append_output(state, "brfalse @skip_%d\n", skipId);
                generate_exp(state, arg->children[2]);
                append_output(state, "st.arg %d\n", argnum);
                append_output(state, "@skip_%d:\npop\n", skipId);
            }
            argnum++;
        }
    }
}
