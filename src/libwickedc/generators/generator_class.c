#include <assert.h>
#include "generator.h"


void generate_extends(generator_state_t *state, mpc_ast_t *ast, const char* class_name) {
    append_debug_setcontext(state, ast);
    generate_ident(state, ast->children[1]);

    int i;
    for (i = 2; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
            append_output(state, "ld.mapitem \"%s\"\n", ast->contents);
        }
    }

    append_output(state, "ld.deref %s\n", class_name);
    append_output(state, "map.setprototype\n");
}

void generate_class(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("class|>", ast->tag));
    append_debug_setcontext(state, ast);

    char *name = ast->children[1]->contents;
    append_output(state, "# Class name: %s\n", name);

    append_output(state, "# alloc %s\n", name);

    if (strcmp(ast->children[2]->tag, "extends|>") == 0) {
        generate_extends(state, ast->children[2], name);
    }

    enter_scope(state, name, NULL, NULL);

    // scan for fields
    for (int i = 3; i < ast->children_num; i++) {
        if (strcmp("classDecl|>", ast->children[i]->tag) == 0) {
            mpc_ast_t *classDecl = ast->children[i];
            mpc_ast_t *decl = classDecl->children[classDecl->children_num - 1];
            if (strcmp(classDecl->children[0]->contents, "static") == 0) {
                state->is_static = 1;
            }
            if (strcmp("function|>", decl->tag) == 0) {
                const char *funcName = decl->children[1]->contents;
                append_output(state, "# function: %s\n", decl->children[1]->contents);
                append_output(state, "ld.ref %s.%s\nld.deref %s\nst.mapitem \"%s\"\n", name, funcName, name, funcName);
            }
            if (strcmp("classVar|>", decl->tag) == 0) {
                mpc_ast_t *classVar = decl;
                for (int j = 1; j < classVar->children_num; j++) {
                    if (strcmp("decl|>", classVar->children[j]->tag) == 0) {
                        const char* varName = classVar->children[j]->children[0]->contents;
                        append_output(state, "# var: %s\n", varName);

                        if (classVar->children[j]->children_num > 1) {
                            // this decl has a def value
                            generate_exp(state, classVar->children[j]->children[2]);
                            append_output(state, "ld.deref %s\n", name);
                            append_output(state, "st.mapitem \"%s\"\n", varName);
                        } else {
                            append_output(state, "ld.deref %s\nld.empty\nst.mapitem \"%s\"\n", name, varName);
                        }
                        append_output(state, "\n");
                    }
                }
            }
            state->is_static = 0;
        }
    }

    for (int i = 3; i < ast->children_num; i++) {
        if (strcmp("classDecl|>", ast->children[i]->tag) == 0) {
            mpc_ast_t *classDecl = ast->children[i];
            mpc_ast_t *decl = classDecl->children[classDecl->children_num - 1];
            if (strcmp(classDecl->children[0]->contents, "static") == 0) {
                state->is_static = 1;
            }
            if (strcmp("function|>", decl->tag) == 0) {
                char prefix[128];
                strcpy(prefix, name);
                strcat(prefix, ".");
                generate_function(state, decl, prefix);
            }
            state->is_static = 0;
        }
    }

    append_output(state, "#allocator\n");
    append_output(state, "jmp @%s__end\n", name);
    append_output(state, "@alloc_%s:\n", name);
    append_debug_enterscope(state, "@alloc_", name);
    append_output(state, "ld.map\n");
    append_output(state, "ld.deref %s\n", name);
    append_output(state, "ld.stack -1\n");
    append_output(state, "map.setprototype\n");
    append_output(state, "ld.uint 1\n");
    append_output(state, "ld.stack -1\n");
    append_output(state, "st.mapitem \"@is_instance\"\n");
    append_output(state, "st.reg %%rr\n");
    append_debug_leavescope(state);
    append_output(state, "ret\n");
    append_output(state, "@%s__end:\n", name);
    append_output(state, "ld.ref @alloc_%s\n", name);
    append_output(state, "ld.deref %s\n", name);
    append_output(state, "st.mapitem \"@alloc\"\n");

    append_output(state, "\n\n");

    leave_scope(state);
}

void generate_newCall(generator_state_t *state, mpc_ast_t *ast) {
    append_debug_setcontext(state, ast);

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

    append_output(state,"ld.stack -%d\n", num_arguments + 1); // the map
    append_output(state,"st.reg %%r1\n", num_arguments); // the map
    append_output(state,"ld.stack -%d\n", num_arguments); // the address of the function
    append_output(state,"call.pop %d\n", num_arguments);
    //append_output(state,"pop\n"); // pop the duplicated func address
}

void generate_new(generator_state_t *state, mpc_ast_t *ast) {
    append_debug_setcontext(state, ast);

    if (state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        fprintf(stderr, "%s:%ld:%ld error: new operator can't be an lvalue\n", state->filename,
                ast->children[0]->state.row + 1, ast->children[0]->state.col + 1);
        exit(EXIT_FAILURE);
    }

    generate_ident(state, ast->children[1]);

    int i;
    for (i = 2; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
            append_output(state, "ld.mapitem \"%s\"\n", ast->children[i]->contents);
        } else if (strcmp(ast->children[i]->tag, "funCall|>") == 0) {
            break;
        }
    }

    // call allocator
    append_output(state,"ld.mapitem \"@alloc\"\n");
    append_output(state,"call.pop 0\n");
    append_output(state,"ld.reg %%rr\n");

    // call initializer
    append_output(state,"dup\n");
    append_output(state,"ld.mapitem \"new\"\n");

    int skipId = state->uniqueid++;
    append_output(state,"is.empty\nbrtrue @skip_%d\n", skipId);

    if (i < ast->children_num && strcmp(ast->children[i]->tag, "funCall|>") == 0) {
        generate_newCall(state, ast->children[i]);
    } else {
        // no funCall, just call bare
        // if no 'new' method, skip
        append_output(state,"ld.stack -1\n"); // the map
        append_output(state,"st.reg %%r1\n"); // the map
        append_output(state,"ld.stack 0\n"); // the address of the function
        append_output(state,"call.pop 0\n");
    }
    append_output(state,"@skip_%d:\npop\n@end_%d:\n", skipId, skipId);
}

void generate_prototypeof(generator_state_t *state, mpc_ast_t *ast) {
    if (state->exp_state->is_lvalue && state->exp_state->is_last_member) {
        fprintf(stderr, "%s:%ld:%ld error: prototypeof can't be an lvalue\n", state->filename,
                ast->children[0]->state.row + 1, ast->children[0]->state.col + 1);
        exit(EXIT_FAILURE);
    }

    if (ast->children_num == 2) {
        if (strcmp(ast->children[1]->contents, "string") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_STRING\n");
            return;
        }

        if (strcmp(ast->children[1]->contents, "int") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_INT\n");
            return;
        }

        if (strcmp(ast->children[1]->contents, "uint") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_UINT\n");
            return;
        }

        if (strcmp(ast->children[1]->contents, "array") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_ARRAY\n");
            return;
        }

        if (strcmp(ast->children[1]->contents, "float") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_FLOAT\n");
            return;
        }

        if (strcmp(ast->children[1]->contents, "map") == 0) {
            append_output(state, "ld.boxingproto VM_TYPE_MAP\n");
            return;
        }
    }

    generate_ident(state, ast->children[1]);

    int i;
    for (i = 2; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "ident|>") == 0) {
            append_output(state, "ld.mapitem \"%s\"\n", ast->children[i]->contents);
        }
    }
}