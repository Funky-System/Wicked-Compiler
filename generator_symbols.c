#include <stdlib.h>
#include <assert.h>
#include "generator.h"

/* Declare type-specific symbol_table_hashmap_* functions with this handy macro */
HASHMAP_FUNCS_CREATE(symbol_table, const char, struct symbol_table_entry)

void generate_ident(generator_state_t *state, mpc_ast_t *ast) {
    append_debug_setcontext(state, ast);

    if (strcmp(ast->contents, "this") == 0) {
        append_output(state,"ld.local 0\n");
        return;
    }

    if (strcmp(ast->contents, "string") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_STRING\n");
        return;
    }

    if (strcmp(ast->contents, "int") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_INT\n");
        return;
    }

    if (strcmp(ast->contents, "uint") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_UINT\n");
        return;
    }

    if (strcmp(ast->contents, "array") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_ARRAY\n");
        return;
    }

    if (strcmp(ast->contents, "float") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_FLOAT\n");
        return;
    }

    if (strcmp(ast->contents, "map") == 0) {
        append_output(state,"ld.boxingproto VM_TYPE_MAP\n");
        return;
    }

    if (state->exp_state != NULL && state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        // this is an assignment to an ident
        if (state->exp_state->is_first_member) {
            // assignment to single ident
            struct symbol_table_entry *entry = get_symbol_from_scopedIdent(state, ast);

            if (entry->type == SYMBOL_TYPE_LOCAL) {
                append_output(state, "st.local %d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
                append_output(state, "st.addr @global_%d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_PARAM) {
                append_output(state, "st.arg %d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is a function, not an lvalue\n", state->filename,
                        ast->state.row + 1,
                        ast->state.col, ast->contents);
                exit(EXIT_FAILURE);
            } else if (entry->type == SYMBOL_TYPE_CLASS) {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is a class, not an lvalue\n", state->filename,
                        ast->state.row + 1,
                        ast->state.col, ast->contents);
                exit(EXIT_FAILURE);
            } else if (entry->type == SYMBOL_TYPE_MODULE) {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is a module, not an lvalue\n", state->filename,
                        ast->state.row + 1,
                        ast->state.col, ast->contents);
                exit(EXIT_FAILURE);
            } else if (entry->type == SYMBOL_TYPE_FIELD) {
                append_output(state, "ld.local 0\nst.mapitem \"%s\"\n", entry->name);
            }
        } else {
            // assignment to member ident
            append_output(state, "st.mapitem \"%s\"\n", ast->contents);
        }
    } else {
        // this is a value accessor
        if (state->exp_state == NULL || state->exp_state->is_first_member) {
            // first or single member
            struct symbol_table_entry *entry = get_symbol_from_scopedIdent(state, ast);

            if (entry->type == SYMBOL_TYPE_LOCAL) {
                append_output(state, "ld.local %d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
                append_output(state, "ld.deref @global_%d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_PARAM) {
                append_output(state, "ld.arg %d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
                append_output(state, "ld.ref %s\n", entry->name);
            } else if (entry->type == SYMBOL_TYPE_CLASS) {
                append_output(state, "ld.deref %s\n", entry->name);
            } else if (entry->type == SYMBOL_TYPE_MODULE) {
                append_output(state, "ld.deref %s\n", entry->name);
            } else if (entry->type == SYMBOL_TYPE_FIELD) {
                append_output(state, "ld.local 0\nld.mapitem \"%s\"\n", entry->name);
            }
        } else {
            // this is not the first or single member
            append_output(state, "ld.mapitem \"%s\"\n", ast->contents);
        }
    }
}

struct symbol_table_entry *get_symbol_from_ident(generator_state_t *state, const char* ident) {
    char *scope = malloc(strlen(state->scope) + 1);
    strcpy(scope, state->scope);

    char* scoped_ident = malloc(strlen(state->scope) + strlen(ident) + 2);
    scoped_ident[0] = '\0';
    if (state->scope[0] != '\0') {
        strcpy(scoped_ident, state->scope);
        strcat(scoped_ident, ".");
    }
    strcat(scoped_ident, ident);

    struct symbol_table_entry *entry = symbol_table_hashmap_get(&state->symbol_table, scoped_ident);
    while (entry == NULL) {
        char *pos = strrchr(scope, '.');
        if(pos == NULL) {
            entry = symbol_table_hashmap_get(&state->symbol_table, ident);
            if (entry == NULL) {
                fprintf(stderr, "%s:%d:%d error: '%s' is not defined\n", state->filename,
                        -1,
                        -1, ident);
                exit(EXIT_FAILURE);
            }
        } else {
            *scoped_ident = '\0';
            strncat(scoped_ident, scope, pos - scope + 1);
            strcat(scoped_ident, scoped_ident);
            pos[0] = '\0';
            entry = symbol_table_hashmap_get(&state->symbol_table, scoped_ident);
        }
    }

    free(scoped_ident);

    return entry;
}

struct symbol_table_entry *get_symbol_from_scopedIdent(generator_state_t *state, mpc_ast_t* identtag) {
    char *scoped_ident = identtag->contents;

    if (str_startswith(scoped_ident, "global.")) {
        struct symbol_table_entry *entry = symbol_table_hashmap_get(&state->symbol_table, scoped_ident + 7);
        if (entry == NULL) {
            fprintf(stderr, "%s:%ld:%ld error: global '%s' is not defined\n", state->filename,
                    identtag->state.row + 1,
                    identtag->state.col, scoped_ident + 7);
            exit(EXIT_FAILURE);
        }
        return entry;
    }

    char *scope = malloc(strlen(state->scope) + 1);
    strcpy(scope, state->scope);
    char* ident = malloc(strlen(scope) + strlen(scoped_ident) + 2);
    ident[0] = '\0';
    if (state->scope[0] != '\0') {
        strcat(ident, scope);
        strcat(ident, ".");
    }
    strcat(ident, scoped_ident);

    struct symbol_table_entry *entry = symbol_table_hashmap_get(&state->symbol_table, ident);
    while (entry == NULL) {
        char *pos = strrchr(scope, '.');
        if(pos == NULL) {
            entry = symbol_table_hashmap_get(&state->symbol_table, scoped_ident);
            if (entry == NULL) {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is not defined\n", state->filename,
                        identtag->children[0]->state.row + 1,
                        identtag->children[0]->state.col, scoped_ident);
                exit(EXIT_FAILURE);
            }
        } else {
            *ident = '\0';
            strncat(ident, scope, pos - scope + 1);
            strcat(ident, scoped_ident);
            pos[0] = '\0';
            entry = symbol_table_hashmap_get(&state->symbol_table, ident);
        }
    }
    free(ident);
    free(scope);

    return entry;
}

void enter_scope(generator_state_t *state, const char* name) {
    if (state->scope[0] != '\0') {
        strcat(state->scope, ".");
    }
    strcat(state->scope, name);
}

void leave_scope(generator_state_t *state) {
    char *pos = strrchr(state->scope, '.');
    if(pos == NULL) {
        if (strcmp(state->scope, "^") == 0) {
            strcpy(state->scope, "");
            return;
        } else {
            pos = state->scope;
        }
    }

    if (strcmp(pos, ".^") != 0) {
        struct hashmap_iter *iter = hashmap_iter(&state->symbol_table);
        char scope[strlen(state->scope) + 2];
        strcpy(scope, state->scope);
        strcat(scope, ".");
        while (iter) {
            struct symbol_table_entry *entry = symbol_table_hashmap_iter_get_data(iter);
            const char *key = symbol_table_hashmap_iter_get_key(iter);
            if (str_startswith(key, scope)) {
                free((void *) key);
                free(entry);
                iter = hashmap_iter_remove(&state->symbol_table, iter);
            } else {
                iter = hashmap_iter_next(&state->symbol_table, iter);
            }
        }
    }

    *pos = '\0';
}

void print_symbol_table(generator_state_t *state) {
    printf("Symbol table:\n");
    for (struct hashmap_iter *iter = hashmap_iter(&state->symbol_table); iter; iter = hashmap_iter_next(&state->symbol_table, iter)) {
        printf("%d '%s': '%s' -> %d\n", symbol_table_hashmap_iter_get_data(iter)->type, symbol_table_hashmap_iter_get_key(iter),
               symbol_table_hashmap_iter_get_data(iter)->name, symbol_table_hashmap_iter_get_data(iter)->index);
    }

    printf("\n");
}