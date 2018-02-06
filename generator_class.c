#include <assert.h>
#include "generator.h"


void generate_class(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("class|>", ast->tag));

    char *name = ast->children[1]->contents;
    append_output(state, "jmp %s__end\n", name);
    append_output(state, "# Class name: %s\n", name);
    append_output(state, "# initializer%s\n", name);
    append_output(state, "%s: \n", name);
    append_output(state, "locals.res 1\n");
    append_output(state, "ld.map\n");
    append_output(state, "st.local 1\n");

    char *scoped_ident = malloc(strlen(state->scope) + strlen(name) + 2);
    if (state->scope[0] == '\0') {
        strcpy(scoped_ident, "");
    } else {
        strcpy(scoped_ident, state->scope);
        strcat(scoped_ident, ".");
    }
    strcat(scoped_ident, name);
    if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
        struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
        e->name = name;
        e->index = 0;
        e->type = SYMBOL_TYPE_CLASS;
        symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
    } else {
        fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                ast->children[1]->state.row + 1,
                ast->children[1]->state.col + 1, ast->children[1]->contents);
        exit(EXIT_FAILURE);
    }

    enter_scope(state, name);

    // scan for fields
    for (int i = 3; i < ast->children_num; i++) {
        if (strcmp("classDecl|>", ast->children[i]->tag) == 0) {
            mpc_ast_t *classDecl = ast->children[i];
            if (strcmp("function|>", classDecl->children[0]->tag) == 0) {
                const char *funcName = classDecl->children[0]->children[1]->contents;
                append_output(state, "# function: %s\n", classDecl->children[0]->children[1]->contents);
                append_output(state, "ld.ref %s\nld.local 1\nst.mapitem \"%s\"\n", funcName, funcName);

                char *scoped_ident = malloc(strlen(state->scope) + strlen(funcName) + 2);
                strcpy(scoped_ident, state->scope);
                strcat(scoped_ident, ".");
                strcat(scoped_ident, funcName);
                if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                    struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                    e->name = funcName;
                    e->index = 0;
                    e->type = SYMBOL_TYPE_FUNCTION;
                    symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                } else {
                    fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                            classDecl->children[0]->state.row + 1,
                            classDecl->children[0]->state.col + 1, classDecl->children[0]->contents);
                    exit(EXIT_FAILURE);
                }

                state->is_method_definition = 1;
                generate_function(state, classDecl->children[0]);
                state->is_method_definition = 0;
            }
            if (strcmp("classVar|>", classDecl->children[0]->tag) == 0) {
                mpc_ast_t *classVar = classDecl->children[0];
                for (int j = 1; j < classVar->children_num; j++) {
                    if (strcmp("decl|>", classVar->children[j]->tag) == 0) {
                        const char* varName = classVar->children[j]->children[0]->contents;
                        append_output(state, "# var: %s\n", varName);

                        if (classVar->children[j]->children_num > 1) {
                            // this decl has a def value
                            generate_exp(state, classVar->children[j]->children[2]);
                            append_output(state, "ld.local 1\n");
                            append_output(state, "st.mapitem \"%s\"\n", varName);
                        } else {
                            append_output(state, "ld.local 1\nld.empty\nst.mapitem \"%s\"\n", varName);
                        }
                        append_output(state, "\n");

                        char *scoped_ident = malloc(strlen(state->scope) + strlen(varName) + 2);
                        strcpy(scoped_ident, state->scope);
                        strcat(scoped_ident, ".");
                        strcat(scoped_ident, varName);
                        if (symbol_table_hashmap_get(&state->symbol_table, scoped_ident) == NULL) {
                            struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                            e->name = varName;
                            e->index = 0;
                            e->type = SYMBOL_TYPE_FIELD;
                            symbol_table_hashmap_put(&state->symbol_table, scoped_ident, e);
                        } else {
                            fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                                    classDecl->children[0]->state.row + 1,
                                    classDecl->children[0]->state.col + 1, classDecl->children[0]->contents);
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        }
    }

    append_output(state, "ld.local 1\n");
    append_output(state, "st.reg %%rr\n");
    append_output(state, "locals.cleanup\n");
    append_output(state, "ret\n");
    append_output(state, "%s__end: ", name);
    append_output(state, "# end\n");

    leave_scope(state);

}
