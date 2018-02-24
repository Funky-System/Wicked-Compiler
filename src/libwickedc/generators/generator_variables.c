
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
            append_output(state,"st.ref @global_%d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_PARAM) {
            append_output(state,"st.arg %d\n", entry->index);
        }
    }
}

void populate_symbol_table(generator_state_t *state, mpc_ast_t *ast, int depth, int *num_locals, int *num_params,
                           enum symbol_type type) {
    int in_data_section = 0;
    if (type == SYMBOL_TYPE_GLOBAL && depth == -1) {
        append_output(state,"section .data\n");
        in_data_section = 1;
        depth = 0;
    }

    if (strcmp("stmt|>", ast->tag) == 0) {
        if (strcmp("var", ast->children[0]->contents) == 0) {
            for (int j = 0; j < ast->children_num; j++) {
                if (strcmp(ast->children[j]->tag, "decl|>") == 0) {
                    char *ident = ast->children[j]->children[0]->contents;
                    char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + 2);
                    scoped_ident[0] = '\0';
                    if (strlen(state->scope) > 0) {
                        strcpy(scoped_ident, state->scope);
                        strcat(scoped_ident, ".");
                    }

                    int d = depth;
                    while (d > 0) {
                        scoped_ident = realloc(scoped_ident, strlen(scoped_ident) + 3 + strlen(ident) + 1);
                        strcat(scoped_ident, "^.");
                        d--;
                    }

                    strcat(scoped_ident, ident);
                    if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                        struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                        e->name = strdup(ident);
                        e->index = *num_locals;
                        e->type = type;
                        symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                        (*num_locals)++;
                        if (type == SYMBOL_TYPE_GLOBAL) append_output(state,"@global_%d: var\n", e->index);
                        free(scoped_ident);
                    } else {
                        fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                                ast->children[0]->state.row + 1,
                                ast->children[0]->state.col, ident);
                        free(scoped_ident);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        } else if (strcmp("for", ast->children[0]->contents) == 0) {
            char *ident = ast->children[1]->contents;
            char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + depth * 2 + 3);
            scoped_ident[0] = '\0';
            if (strlen(state->scope) > 0) {
                strcpy(scoped_ident, state->scope);
                strcat(scoped_ident, ".");
            }

            int d = depth + 1;
            while (d > 0) {
                scoped_ident = realloc(scoped_ident, strlen(scoped_ident) + 3 + strlen(ident) + 1);
                strcat(scoped_ident, "^."), d--;
            }

            strcat(scoped_ident, ident);
            if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                e->name = strdup(ident);
                e->index = *num_locals;
                e->type = type;
                symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                (*num_locals)++;
                if (type == SYMBOL_TYPE_GLOBAL) append_output(state,"@global_%d: var\n", e->index);
                free(scoped_ident);
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                        ast->children[0]->state.row + 1,
                        ast->children[0]->state.col, ast->children[0]->contents);
                free(scoped_ident);
                exit(EXIT_FAILURE);
            }
        }
    } else if (strcmp("args|>", ast->tag) == 0) {
        *num_params = 0;
        for (int i = 0; i < ast->children_num; i++) {
            if (strcmp(ast->children[i]->tag, "arg|>") == 0) {
                (*num_params)++;
            }
        }
        int cur_param = 0;
        for (int i = 0; i < ast->children_num; i++) {
            if (strcmp(ast->children[i]->tag, "arg|>") == 0) {
                char *ident = ast->children[i]->children[0]->contents;
                char *scoped_ident = malloc(strlen(state->scope) + strlen(ident) + 2);
                strcpy(scoped_ident, state->scope);
                strcat(scoped_ident, ".");
                strcat(scoped_ident, ident);
                if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                    struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                    e->name = strdup(ident);
                    e->index = cur_param;
                    e->type = SYMBOL_TYPE_PARAM;
                    symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                    cur_param++;
                    free(scoped_ident);
                } else {
                    fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                            ast->children[i]->children[0]->state.row + 1,
                            ast->children[i]->children[0]->state.col, ast->children[i]->children[0]->contents);
                    free(scoped_ident);
                    exit(EXIT_FAILURE);
                }
            }
        }
    } else if (type == SYMBOL_TYPE_GLOBAL && strcmp("function|>", ast->tag) == 0) {
        char *ident = ast->children[1]->contents;
        if (strcmp(ast->children[1]->tag, "syscallAlias|>") == 0) {
            ident = ast->children[2]->contents;
        }
        if (symbol_table_hashmap_get(&state->symbol_table, ident) == NULL) {
            struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
            e->name = strdup(ident);
            e->index = 0;
            e->type = SYMBOL_TYPE_FUNCTION;
            symbol_table_hashmap_put(&state->symbol_table, ident, e);
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                    ast->children[0]->state.row + 1,
                    ast->children[0]->state.col, ident);
            exit(EXIT_FAILURE);
        }
    } else if (type == SYMBOL_TYPE_GLOBAL && strcmp("class|>", ast->tag) == 0) {
        char *ident = ast->children[1]->contents;
        if (symbol_table_hashmap_get(&state->symbol_table, ident) == NULL) {

            struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
            e->name = strdup(ast->children[1]->contents);
            e->index = 0;
            e->type = SYMBOL_TYPE_CLASS;
            symbol_table_hashmap_put(&state->symbol_table, ident, e);

            append_output(state, "%s: var\n", ident);
            append_output(state, "section .text\n");
            append_output(state, "ld.map\nst.ref %s\n", ident);
            append_output(state, "section .data\n");

        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                    ast->children[0]->state.row + 1,
                    ast->children[0]->state.col, ast->children[0]->contents);
            exit(EXIT_FAILURE);
        }
    } else if (type == SYMBOL_TYPE_GLOBAL && strcmp("enum|>", ast->tag) == 0) {
        char *ident = ast->children[1]->contents;
        if (symbol_table_hashmap_get(&state->symbol_table, ident) == NULL) {

            struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
            e->name = strdup(ast->children[1]->contents);
            e->index = 0;
            e->type = SYMBOL_TYPE_ENUM;
            symbol_table_hashmap_put(&state->symbol_table, ident, e);

            append_output(state, "%s: var\n", ident);
            append_output(state, "section .text\n");
            append_output(state, "ld.map\nst.ref %s\n", ident);
            append_output(state, "section .data\n");

        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                    ast->children[0]->state.row + 1,
                    ast->children[0]->state.col, ast->children[0]->contents);
            exit(EXIT_FAILURE);
        }
    }

    if (type == SYMBOL_TYPE_GLOBAL && strcmp(ast->tag, "function|>") == 0) {
        return;
    }

    if (type == SYMBOL_TYPE_GLOBAL && strcmp(ast->tag, "class|>") == 0) {
        return;
    }

    for (int i = 0; i < ast->children_num; i++) {
        int sub_depth = depth;
        if (strcmp(ast->tag, "stmt|>") == 0) sub_depth++;
        populate_symbol_table(state, ast->children[i], sub_depth, num_locals, num_params, type);
    }

    if (in_data_section) append_output(state,"\nsection .text\n");
}
