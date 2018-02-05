
#include <assert.h>
#include "generator.h"

void generate_decl(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("decl|>", ast->tag));

    if (ast->children_num > 1) {
        // assignment
        assert(0 == strcmp("=", ast->children[1]->contents));
        generate_exp(state, ast->children[2]);
        struct symbol_table_entry *entry = get_symbol_from_ident(state, ast->children[0]->contents);
        if (entry->type == SYMBOL_TYPE_LOCAL) {
            append_output(state,"st.local %d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
            append_output(state,"st.addr global_%d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_PARAM) {
            append_output(state,"st.arg %d\n", entry->index);
        }
    }
}

void reserve_globals(generator_state_t *state, mpc_ast_t *ast, int depth, int *num_globals) {
    if (depth == -1) append_output(state,"section .data\n");

    if (strcmp("stmt|>", ast->tag) == 0) {
        if (strcmp("var", ast->children[0]->contents) == 0) {
            for (int j = 0; j < ast->children_num; j++) {
                if (strcmp(ast->children[j]->tag, "decl|>") == 0) {
                    char *ident = ast->children[j]->children[0]->contents;
                    char *scoped_ident = malloc(strlen(ident) + 1 + depth * 2);

                    int d = depth;
                    while (d > 0) strcat(scoped_ident, "^."), d--;

                    strcat(scoped_ident, ident);
                    if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                        struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                        e->name = ident;
                        e->index = *num_globals;
                        e->type = SYMBOL_TYPE_GLOBAL;
                        symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                        (*num_globals)++;
                        append_output(state,"global_%d: var\n", e->index);
                    } else {
                        fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                                ast->children[0]->state.row + 1,
                                ast->children[0]->state.col + 1, ast->children[0]->contents);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        if (strcmp("for", ast->children[0]->contents) == 0) {
            char *ident = ast->children[1]->contents;
            char *scoped_ident = malloc(strlen(ident) + 1 + depth * 2);

            int d = depth + 1;
            while (d > 0) strcat(scoped_ident, "^."), d--;

            strcat(scoped_ident, ident);
            if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                e->name = ident;
                e->index = *num_globals;
                e->type = SYMBOL_TYPE_GLOBAL;
                symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                (*num_globals)++;
                append_output(state,"global_%d: var\n", e->index);
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                        ast->children[0]->state.row + 1,
                        ast->children[0]->state.col + 1, ast->children[0]->contents);
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp("function|>", ast->tag) == 0) {
        char *ident = ast->children[1]->contents;
        if (symbol_table_hashmap_get(&state->symbol_table, ident) == NULL) {

            struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
            e->name = ast->children[1]->contents;
            e->index = 0;
            e->type = SYMBOL_TYPE_FUNCTION;
            symbol_table_hashmap_put(&state->symbol_table, ident, e);
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                    ast->children[0]->state.row + 1,
                    ast->children[0]->state.col + 1, ast->children[0]->contents);
            exit(EXIT_FAILURE);
        }
    }

    if (strcmp(ast->tag, "function|>") == 0) {
        return;
    }

    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "stmt|>") == 0 || strcmp(ast->children[i]->tag, "function|>") == 0) {
            reserve_globals(state, ast->children[i], depth + 1, num_globals);
        }
    }

    if (depth == -1) append_output(state,"\nsection .text\n");
}