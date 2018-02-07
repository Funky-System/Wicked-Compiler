#include <assert.h>
#include "generator.h"

void reserve_locals(generator_state_t *state, mpc_ast_t *ast, int depth, int *num_locals, int *num_params) {
    if (strcmp("stmt|>", ast->tag) == 0) {
        if (strcmp("var", ast->children[0]->contents) == 0) {
            for (int j = 0; j < ast->children_num; j++) {
                if (strcmp(ast->children[j]->tag, "decl|>") == 0) {
                    char *ident = ast->children[j]->children[0]->contents;
                    char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + 2);
                    strcpy(scoped_ident, state->scope);
                    strcat(scoped_ident, ".");

                    int d = depth;
                    while (d > 0) strcat(scoped_ident, "^."), d--;

                    strcat(scoped_ident, ident);
                    if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                        struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                        e->name = ident;
                        e->index = *num_locals + 1;
                        e->type = SYMBOL_TYPE_LOCAL;
                        symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                        (*num_locals)++;
                    }
                }
            }
        } else if (strcmp("for", ast->children[0]->contents) == 0) {
            char *ident = ast->children[1]->contents;
            char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + depth * 2 + 2);
            strcpy(scoped_ident, state->scope);
            strcat(scoped_ident, ".");

            int d = depth + 1;
            while (d > 0) strcat(scoped_ident, "^."), d--;

            strcat(scoped_ident, ident);
            if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                e->name = ident;
                e->index = *num_locals + 1;
                e->type = SYMBOL_TYPE_LOCAL;
                symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                (*num_locals)++;
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                        ast->children[0]->state.row + 1,
                        ast->children[0]->state.col + 1, ast->children[0]->contents);
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp("args|>", ast->tag) == 0) {
        *num_params = 0;
        for (int i = 0; i < ast->children_num; i++) {
            if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
                (*num_params)++;
            }
        }
        int cur_param = 0;
        for (int i = 0; i < ast->children_num; i++) {
            if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
                char *ident = ast->children[i]->contents;
                char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + 2);
                strcpy(scoped_ident, state->scope);
                strcat(scoped_ident, ".");
                strcat(scoped_ident, ident);
                if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                    struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                    e->name = ident;
                    e->index = cur_param;
                    e->type = SYMBOL_TYPE_PARAM;
                    symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                    cur_param++;
                } else {
                    fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                            ast->children[i]->state.row + 1,
                            ast->children[i]->state.col + 1, ast->children[i]->contents);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    for (int i = 0; i < ast->children_num; i++) {
        int sub_depth = depth;
        if (strcmp(ast->tag, "stmt|>") == 0) sub_depth++;
        reserve_locals(state, ast->children[i], sub_depth, num_locals, num_params);
    }
}

void generate_function(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("function|>", ast->tag));

    char *name = ast->children[1]->contents;
    append_output(state,"# Function name: %s\n", name);
    append_output(state,"jmp %s__end\n", name);
    append_output(state,"%s: ", name);

    enter_scope(state, name);

    // scan for locals
    int num_locals = 0, num_params = 0;
    reserve_locals(state, ast, 0, &num_locals, &num_params);
    if (state->is_method_definition) {
        num_params++;
    }
    state->function_num_params = num_params;
    append_output(state,"args.accept %d\n", num_params);
    append_output(state,"locals.res %d\n", num_locals);

    int index = mpc_ast_get_index(ast, "stmt|>");
    if (index == -1) index = ast->children_num - 2;

    for (int i = index; i < ast->children_num; i++) {
        if (strcmp("stmt|>", ast->children[i]->tag) == 0) {
            generate_stmt(state, ast->children[i]);
        } else if (strcmp("string", ast->children[i]->tag) == 0 && strcmp("end", ast->children[i]->contents) == 0) {
            append_output(state,"locals.cleanup\nargs.cleanup\nld.int 0\nst.reg %%rr\nret\n");
            append_output(state,"%s__end: ", name);
            append_output(state,"# end\n");
        }
    }

    append_output(state, "\n");
    leave_scope(state);
}
