#include <stdlib.h>
#include <assert.h>
#include "generator.h"

/* Declare type-specific symbol_table_hashmap_* functions with this handy macro */
HASHMAP_FUNCS_CREATE(symbol_table, const char, struct symbol_table_entry)

void generate_ident(generator_state_t *state, mpc_ast_t *ast) {
    append_debug_setcontext(state, ast);

    if (strcmp(ast->contents, "this") == 0) {
        if (state->is_static) {
            fprintf(stderr, "%s:%ld:%ld error: 'this' is not allowed in static context\n", state->filename,
                    ast->state.row + 1,
                    ast->state.col);
            exit(EXIT_FAILURE);
        } else if (state->exp_state != NULL && state->exp_state->is_lvalue && state->exp_state->is_last_member) {
            fprintf(stderr, "%s:%ld:%ld error: 'this' is not an lvalue\n", state->filename,
                    ast->state.row + 1,
                    ast->state.col);
            exit(EXIT_FAILURE);
        } else {
            append_output(state,"ld.local 0\n");
            //append_output(state,"is.map\nbrtrue @skip_%d\nbox\n@skip_%d:\n", skipId, skipId);
        }

        return;
    }

    if (strcmp(ast->contents, "empty") == 0) {
        append_output(state,"ld.empty\n");
        return;
    }

    if (strcmp(ast->contents, "true") == 0) {
        append_output(state,"ld.uint 1\n");
        return;
    }

    if (strcmp(ast->contents, "false") == 0) {
        append_output(state,"ld.uint 0\n");
        return;
    }

    if (state->exp_state != NULL && state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        // this is an assignment to an ident
        if (state->exp_state->is_first_member) {
            // assignment to single ident
            struct symbol_table_entry *entry = get_symbol_from_scopedIdent(state, ast);

            append_output(state, "dup\n");

            if (entry->type == SYMBOL_TYPE_LOCAL) {
                append_output(state, "st.local %d\n", entry->index);
            } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
                append_output(state, "st.ref @global_%d\n", entry->index);
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
            } else if (entry->type == SYMBOL_TYPE_ENUM) {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is an enum, not an lvalue\n", state->filename,
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
            append_output(state, "ld.stack -1\nst.reg %%r2\n");
            append_output(state, "st.mapitem \"%s\"\n", ast->contents);
            append_output(state, "ld.reg %%r2\n");
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
            } else if (entry->type == SYMBOL_TYPE_ENUM) {
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
            strcat(scoped_ident, ident);
            pos[0] = '\0';
            entry = symbol_table_hashmap_get(&state->symbol_table, scoped_ident);
        }
    }

    free(scoped_ident);
    free(scope);

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
                        identtag->state.row + 1,
                        identtag->state.col, scoped_ident);
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

void enter_scope(generator_state_t *state, const char* name, const char* continue_label, const char* break_label) {
    if (state->scope[0] != '\0') {
        strcat(state->scope, ".");
    }
    strcat(state->scope, name);

    state->num_continue_labels++;
    state->continue_labels = realloc(state->continue_labels, state->num_continue_labels * sizeof(char *));
    if (continue_label != NULL) {
        state->continue_labels[state->num_continue_labels - 1] = strdup(continue_label);
    } else {
        state->continue_labels[state->num_continue_labels - 1] = NULL;
    }

    state->num_break_labels++;
    state->break_labels = realloc(state->break_labels, state->num_break_labels * sizeof(char *));
    if (break_label != NULL) {
        state->break_labels[state->num_break_labels - 1] = strdup(break_label);
    } else {
        state->break_labels[state->num_break_labels - 1] = NULL;
    }
}

void enter_scope_with_pos(generator_state_t *state, const char* name, long pos, const char* continue_label, const char* break_label) {
    char *scope = malloc(strlen(name) + 64);
    sprintf(scope, "%s%ld", name, pos);
    enter_scope(state, scope, continue_label, break_label);
    free(scope);
}

void leave_scope(generator_state_t *state, int cleanup) {
    state->num_continue_labels--;
    if (state->continue_labels[state->num_continue_labels] != NULL) {
        free(state->continue_labels[state->num_continue_labels]);
    }
    state->continue_labels = realloc(state->continue_labels, state->num_continue_labels * sizeof(char*));

    state->num_break_labels--;
    if (state->break_labels[state->num_break_labels] != NULL) {
        free(state->break_labels[state->num_break_labels]);
    }
    state->break_labels = realloc(state->break_labels, state->num_break_labels * sizeof(char*));

    char *pos = strrchr(state->scope, '.');
    if(pos == NULL) {
        if (state->scope[0] == '^') {
            strcpy(state->scope, "");
            return;
        } else {
            pos = state->scope;
        }
    }

    if (cleanup) {
        struct hashmap_iter *iter = hashmap_iter(&state->symbol_table);
        char *scope = malloc(strlen(state->scope) + 2);
        strcpy(scope, state->scope);
        strcat(scope, ".");
        while (iter) {
            struct symbol_table_entry *entry = symbol_table_hashmap_iter_get_data(iter);
            const char *key = symbol_table_hashmap_iter_get_key(iter);
            if (str_startswith(key, scope)) {
                free(entry->name);
                free(entry);
                iter = hashmap_iter_remove(&state->symbol_table, iter);
            } else {
                iter = hashmap_iter_next(&state->symbol_table, iter);
            }
        }
        free(scope);
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