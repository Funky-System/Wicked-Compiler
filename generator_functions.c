#include <assert.h>
#include "generator.h"

void generate_default_params(generator_state_t *state, mpc_ast_t *ast);

void generate_function(generator_state_t *state, mpc_ast_t *ast, const char* prefix) {
    assert(0 == strcmp("function|>", ast->tag));

    char *name = ast->children[1]->contents;
    append_output(state,"# Function name: %s%s\n", prefix, name);
    append_output(state,"jmp @%s%s__end\n", prefix, name);
    append_output(state,"%s%s: \n", prefix, name);

    append_debug_enterscope(state, prefix, name);

    //char *prefix_plus_name = malloc(strlen(prefix) + strlen(name) + 1);
    //strcpy(prefix_plus_name, prefix); strcat(prefix_plus_name, name);
    enter_scope(state, name);

    // scan for locals
    int num_locals = 1 /* 'this' is the first local */, num_params = 0;
    populate_symbol_table(state, ast, 0, &num_locals, &num_params, SYMBOL_TYPE_LOCAL);
    //printf("symbol table after locals for function %s%s:\n", prefix, name);
    //print_symbol_table(state);
    append_output(state,"args.accept %d\n", num_params);
    append_output(state,"locals.res %d\n", num_locals);
    if (state->is_method_definition) {
        append_output(state, "ld.reg %%r1\n");
        append_output(state, "st.local 0\n");
    } else {
        append_output(state, "ld.empty\n");
        append_output(state, "st.local 0\n");
    }

    generate_default_params(state, ast->children[3]);

    int index = mpc_ast_get_index(ast, "stmt|>");
    if (index == -1) index = ast->children_num - 2;

    for (int i = index; i < ast->children_num; i++) {
        if (strcmp("stmt|>", ast->children[i]->tag) == 0) {
            generate_stmt(state, ast->children[i]);
        } else if (strcmp("string", ast->children[i]->tag) == 0 && strcmp("end", ast->children[i]->contents) == 0) {
            append_output(state,"locals.cleanup\nargs.cleanup\nld.int 0\nst.reg %%rr\n");
            append_debug_leavescope(state);
            append_output(state,"ret\n");
            append_output(state,"@%s%s__end: ", prefix, name);
            append_output(state,"# end\n");
        }
    }

    append_output(state, "\n");
    leave_scope(state);
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
