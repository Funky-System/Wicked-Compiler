#include <assert.h>
#include "generator.h"


void generate_extends(generator_state_t *state, mpc_ast_t *ast, const char* class_name) {

    generate_ident(state, ast->children[1]);

    int i;
    for (i = 2; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
            append_output(state, "ld.mapitem \"%s\"\n", ast->contents);
        }
    }

    append_output(state, "call.pop 0\n");
    append_output(state, "ld.reg %%rr\n");
    append_output(state, "map.getprototype\n");
    append_output(state, "ld.deref proto_%s\n",class_name);
    append_output(state, "map.setprototype\n");

}

void generate_class(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("class|>", ast->tag));

    char *name = ast->children[1]->contents;
    append_output(state, "# Class name: %s\n", name);

    append_output(state, "# alloc %s\n", name);

    if (strcmp(ast->children[2]->tag, "extends|>") == 0) {
        generate_extends(state, ast->children[2], name);
    }

    enter_scope(state, name);

    // scan for fields
    for (int i = 3; i < ast->children_num; i++) {
        if (strcmp("classDecl|>", ast->children[i]->tag) == 0) {
            mpc_ast_t *classDecl = ast->children[i];
            if (strcmp("function|>", classDecl->children[0]->tag) == 0) {
                const char *funcName = classDecl->children[0]->children[1]->contents;
                append_output(state, "# function: %s\n", classDecl->children[0]->children[1]->contents);
                append_output(state, "ld.ref %s.%s\nld.deref proto_%s\nst.mapitem \"%s\"\n", name, funcName, name, funcName);

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
                char prefix[128];
                strcpy(prefix, name);
                strcat(prefix, ".");
                generate_function(state, classDecl->children[0], prefix);
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
                            append_output(state, "ld.deref proto_%s\n", name);
                            append_output(state, "st.mapitem \"%s\"\n", varName);
                        } else {
                            append_output(state, "ld.deref proto_%s\nld.empty\nst.mapitem \"%s\"\n", name, varName);
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

    append_output(state, "#allocator\n");
    append_output(state, "jmp %s__end\n", name);
    append_output(state, "%s:\n", name);
    append_output(state, "ld.map\n");
    append_output(state, "ld.deref proto_%s\n", name);
    append_output(state, "ld.stack -1\n");
    append_output(state, "map.setprototype\n");
    append_output(state, "st.reg %%rr\n");
    append_output(state, "ret\n");
    append_output(state, "%s__end:\n", name);

    append_output(state, "\n\n");

    leave_scope(state);

}

void generate_newCall(generator_state_t *state, mpc_ast_t *ast) {
    int i = 1;
    int num_arguments = 0;
    while (strcmp(ast->children[i]->contents, ")") != 0) {
        if (strcmp(ast->children[i]->contents, ",") == 0) {
            i++;
            continue;
        }

        assert(strcmp(ast->children[i]->tag, "exp|>") == 0);
        generate_exp(state, ast->children[i]);
        num_arguments++;

        i++;
    }

    num_arguments++;
    append_output(state,"ld.stack -%d\n", num_arguments); // the map
    append_output(state,"ld.stack -%d\n", num_arguments); // the address of the function
    append_output(state,"call.pop %d\n", num_arguments);
    append_output(state,"pop\n"); // pop the duplicated func address
    append_output(state,"ld.reg %%rr\n");
}

void generate_new(generator_state_t *state, mpc_ast_t *ast) {
    if (state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        fprintf(stderr, "%s:%ld:%ld error: new operator can't be an lvalue\n", state->filename,
                ast->children[0]->state.row + 1, ast->children[0]->state.col + 1);
        exit(EXIT_FAILURE);
    }

    generate_ident(state, ast->children[1]);

    int i;
    for (i = 2; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
            append_output(state, "ld.mapitem \"%s\"\n", ast->contents);
        } else if (strcmp(ast->children[i]->tag, "funCall|>") == 0) {
            break;
        }
    }

    // call allocator
    append_output(state,"call.pop 0\n");
    append_output(state,"ld.reg %%rr\n");

    // call initializer
    append_output(state,"dup\n");
    append_output(state,"ld.mapitem \"new\"\n");

    int skipId = state->uniqueid++;
    append_output(state,"is.empty\nbrtrue skip_%d\n", skipId);

    if (strcmp(ast->children[i]->tag, "funCall|>") == 0) {
        generate_newCall(state, ast->children[i]);
    } else {
        // no funCall, just call bare
        // if no 'new' method, skip
        append_output(state,"call.pop 0\n");
        append_output(state,"ld.reg %%rr\n");
        append_output(state,"jmp end_%d\n", skipId);
    }
    append_output(state,"skip_%d:\npop\nend_%d:\n", skipId, skipId);
}