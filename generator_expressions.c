#include <stdlib.h>
#include <assert.h>
#include "generator.h"

#define tag_startswith(ast, tagstr) (str_startswith(ast->tag, tagstr))

void generate_ident(generator_state_t *state, mpc_ast_t *ast) {
    if (strcmp(ast->contents, "this") == 0) {
        append_output(state,"ld.arg %d\n", state->function_num_params - 1);
        return;
    }

    struct symbol_table_entry *entry = get_symbol_from_scopedIdent(state, ast);

    if (state->is_lvalue) {
        // assignment
        if (entry->type == SYMBOL_TYPE_LOCAL) {
            append_output(state,"st.local %d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
            append_output(state,"st.addr global_%d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_PARAM) {
            append_output(state,"st.arg %d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is a function, not an lvalue\n", state->filename,
                    ast->state.row + 1,
                    ast->state.col + 1, ast->contents);
            exit(EXIT_FAILURE);
        } else if (entry->type == SYMBOL_TYPE_FIELD) {
            append_output(state,"ld.arg %d\nst.mapitem \"%s\"\n", state->function_num_params - 1,entry->name);
        }
    } else {
        // load
        if (entry->type == SYMBOL_TYPE_LOCAL) {
            append_output(state,"ld.local %d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_GLOBAL) {
            append_output(state,"ld.deref global_%d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_PARAM) {
            append_output(state,"ld.arg %d\n", entry->index);
        } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
            append_output(state,"ld.ref %s\n", entry->name);
        } else if (entry->type == SYMBOL_TYPE_CLASS) {
            append_output(state,"ld.ref %s\n", entry->name);
        } else if (entry->type == SYMBOL_TYPE_FIELD) {
            append_output(state,"ld.arg %d\nld.mapitem \"%s\"\n", state->function_num_params - 1,entry->name);
        }
    }
}

void generate_factor(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("factor|>", ast->tag));

    if (ast->children_num == 1) {
        // simple literal or reference
        if (strcmp(ast->children[0]->tag, "int|>") == 0) {
            append_output(state,"ld.int %s\n", ast->children[0]->contents);
        } else if (strcmp(ast->children[0]->tag, "float|>") == 0) {
            append_output(state,"ld.float %s\n", ast->children[0]->contents);
        } else  if (strcmp(ast->children[0]->tag, "character|>") == 0) {
            append_output(state,"ld.uint %s\n", ast->children[0]->contents);
        } else  if (strcmp(ast->children[0]->tag, "stringlit|>") == 0) {
            append_output(state,"ld.str %s\n", ast->children[0]->contents);
        } else if (strcmp(ast->children[0]->tag, "arrayInit|>") == 0) {
            generate_arrayInit(state, ast->children[0]);
        } else {
            assert(0);
        }
    } else if (strcmp(ast->children[0]->contents, "(") == 0) {
        // parentheses
        generate_exp(state, ast->children[1]);
    }
}

void generate_prec20(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec20|>", ast->tag));

    if (strcmp("ident|>", ast->children[0]->tag) == 0) {
        generate_ident(state, ast->children[0]);
    } if (strcmp("ident|>", ast->tag) == 0) {
        generate_ident(state, ast);
    } else if (strcmp("factor|>", ast->children[0]->tag) == 0) {
        generate_factor(state, ast->children[0]);
    }
}

void generate_prec19arr(generator_state_t *state, mpc_ast_t *ast) {
    int old_is_lvalue = state->is_lvalue;
    state->is_lvalue = 0;
    if (strcmp("ident|>", ast->children[0]->tag) == 0) {
        generate_ident(state, ast->children[0]);
    } else {
        generate_prec20(state, ast->children[0]);
    }

    for (int i = 1; i < ast->children_num - 1; i++) {
        generate_arrayIndex(state, ast->children[i]);
    }

    state->is_lvalue = old_is_lvalue;
    generate_arrayIndex(state, ast->children[ast->children_num - 1]);
}

void generate_prec19fun(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec19fun|>", ast->tag));

    int i = 1;
    while (strcmp(ast->children[i]->contents, "(") != 0) i++;

    i++;
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

    generate_ident(state, ast->children[0]->children[0]); // puts function address on the stack

    append_output(state,"call.pop %d\n", num_arguments);
    append_output(state,"ld.reg %%rr\n");
}

void generate_prec19memfun(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec19fun|>", ast->tag));

    append_output(state, "st.reg %%r0\n");

    int i = 1;
    while (strcmp(ast->children[i]->contents, "(") != 0) i++;

    i++;
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
    append_output(state, "ld.reg %%r0\ndup\n");
    append_output(state, "ld.mapitem \"%s\"\n", ast->children[0]->contents);

    append_output(state,"call.pop %d\n", num_arguments);
    append_output(state,"ld.reg %%rr\n");
}

void generate_prec19mem(generator_state_t *state, mpc_ast_t *ast) {
    mpc_ast_t *parent = ast->children[0];
    mpc_ast_t *child = ast->children[2];

    // I assume the parent is a map

    generate_prec20(state, parent);
    if (tag_startswith(child, "prec19fun")) {
        generate_prec19memfun(state, child);
    } else if (tag_startswith(child, "prec19arr")) {
        assert(0);
    } else if (tag_startswith(child, "prec19mem")) {
        generate_prec19mem(state, child);
    } else if (tag_startswith(child, "ident")) {
        if (state->is_lvalue) {
            append_output(state, "st.mapitem \"%s\"\n", child->children[0]->contents);
        } else {
            append_output(state, "ld.mapitem \"%s\"\n", child->children[0]->contents);
        }
    } else {
        assert(0);
    }


    // TODO
}

void generate_prec19(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec19|>", ast->tag));

    state->is_latest_prec19 = 0;

    if (tag_startswith(ast->children[0], "prec19arr")) {
        // for now, always latest, but TODO for future
        state->is_latest_prec19 = 1;
        generate_prec19arr(state, ast->children[0]);
    } else if (tag_startswith(ast->children[0], "prec19fun")) {
        // for now, always latest, but TODO for future
        state->is_latest_prec19 = 1;
        generate_prec19fun(state, ast->children[0]);
    } else if (tag_startswith(ast->children[0], "prec19mem")) {
        generate_prec19mem(state, ast->children[0]);
    } else if (tag_startswith(ast->children[0], "prec20")) {
        generate_prec20(state, ast->children[0]);
    }

}

void generate_prec18(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec18|>", ast->tag));

    generate_prec19(state, ast->children[0]);
}


void generate_prec17(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec17|>", ast->tag));

    generate_prec18(state, ast->children[ast->children_num - 1]);
    for (int i = 0; i < ast->children_num - 1; i++) {
        char *oper = ast->children[i]->contents;
        if (strcmp(oper, "++") == 0) {
            // TODO
        } else if (strcmp(oper, "--") == 0) {
            // TODO
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec16(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec16|>", ast->tag));

    generate_prec17(state, ast->children[ast->children_num - 1]);
    for (int i = ast->children_num - 2; i >= 0; i--) {
        char *oper = ast->children[i]->contents;
        if (strcmp(oper, "!") == 0) {
            append_output(state,"not\n"); // TODO: implement proper logical not
        } else if (strcmp(oper, "~") == 0) {
            append_output(state,"not\n");
        } else if (strcmp(oper, "+") == 0) {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is not implemented yet\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
            // TODO
        } else if (strcmp(oper, "-") == 0) {
            append_output(state,"neg\n");
        } else if (strcmp(oper, "++") == 0) {
            // TODO
        } else if (strcmp(oper, "--") == 0) {
            // TODO
        } else if (strcmp(oper, "typeof") == 0) {
            // TODO
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec15(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec15|>", ast->tag));

    generate_prec16(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_exp(state, ast->children[i + 1]);
        if (strcmp(oper, "**") == 0) {
            append_output(state,"pow\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec14(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec14|>", ast->tag));

    generate_prec15(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec15(state, ast->children[i + 1]);
        if (strcmp(oper, "*") == 0) {
            append_output(state,"mul\n");
        } else if (strcmp(oper, "/") == 0) {
            append_output(state,"div\n");
        } else if (strcmp(oper, "%") == 0) {
            append_output(state,"mod\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec13(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec13|>", ast->tag));

    generate_prec14(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec14(state, ast->children[i + 1]);
        if (strcmp(oper, "+") == 0) {
            append_output(state,"add\n");
        } else if (strcmp(oper, "-") == 0) {
            append_output(state,"sub\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec12(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec12|>", ast->tag));

    generate_prec13(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec13(state, ast->children[i + 1]);
        if (strcmp(oper, "<<") == 0) {
            append_output(state,"lsh\n");
        } else if (strcmp(oper, ">>") == 0) {
            append_output(state,"rsh\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec11(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec11|>", ast->tag));

    generate_prec12(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec12(state, ast->children[i + 1]);
        if (strcmp(oper, "<") == 0) {
            append_output(state,"lt\n");
        } else if (strcmp(oper, "<=") == 0) {
            append_output(state,"le\n");
        } else if (strcmp(oper, ">") == 0) {
            append_output(state,"gt\n");
        } else if (strcmp(oper, "ge") == 0) {
            append_output(state,"ge\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec10(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec10|>", ast->tag));

    generate_prec11(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec11(state, ast->children[i + 1]);
        if (strcmp(oper, "==") == 0) {
            append_output(state,"eq.id\n");
        } else if (strcmp(oper, "!=") == 0) {
            append_output(state,"ne.id\n");
        } else if (strcmp(oper, "~==") == 0) {
            append_output(state,"eq\n");
        } else if (strcmp(oper, "!~=") == 0) {
            append_output(state,"ne\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec09(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec09|>", ast->tag));

    generate_prec10(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec10(state, ast->children[i + 1]);
        if (strcmp(oper, "&") == 0) {
            append_output(state,"and\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec08(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec08|>", ast->tag));

    generate_prec09(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec09(state, ast->children[i + 1]);
        if (strcmp(oper, "|") == 0) {
            append_output(state,"or\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec07(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec07|>", ast->tag));

    generate_prec08(state, ast->children[0]);
    for (int i = 1; i < ast->children_num; i += 2) {
        char *oper = ast->children[i]->contents;
        generate_prec08(state, ast->children[i + 1]);
        if (strcmp(oper, "|") == 0) {
            append_output(state,"or\n");
        } else {
            fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename, ast->children[i]->state.row+1,
                    ast->children[i]->state.col + 1, ast->children[i]->contents);
            exit(EXIT_FAILURE);
        }
    }
}

void generate_prec06(generator_state_t *state, mpc_ast_t *ast) {
    // assert(0 == strcmp("prec06|>", ast->tag));
    generate_prec07(state, ast->children[0]);

    if (ast->children_num > 1) {
        int andnum = state->uniqueid++;

        for (int i = 1; i < ast->children_num; i += 2) {
            append_output(state,"dup\nbrfalse endand_%d\n", andnum);
            char *oper = ast->children[i]->contents;
            if (strcmp(oper, "&&") == 0) {
                generate_prec07(state, ast->children[i + 1]);
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename,
                        ast->children[i]->state.row + 1,
                        ast->children[i]->state.col + 1, ast->children[i]->contents);
                exit(EXIT_FAILURE);
            }
        }

        append_output(state,"endand_%d:\n", andnum);
    }
}

void generate_prec05(generator_state_t *state, mpc_ast_t *ast) {
    // assert(0 == strcmp("prec05|>", ast->tag));
    generate_prec06(state, ast->children[0]);

    if (ast->children_num > 1) {
        int ornum = state->uniqueid++;

        for (int i = 1; i < ast->children_num; i += 2) {
            append_output(state,"dup\nbrtrue endor_%d\n", ornum);
            char *oper = ast->children[i]->contents;
            if (strcmp(oper, "||") == 0) {
                generate_prec06(state, ast->children[i + 1]);
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is an unknown operator\n", state->filename,
                        ast->children[i]->state.row + 1,
                        ast->children[i]->state.col + 1, ast->children[i]->contents);
                exit(EXIT_FAILURE);
            }
        }

        append_output(state,"endor_%d:\n", ornum);
    }
}

void generate_prec04(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec04|>", ast->tag));

    generate_prec05(state, ast->children[0]);

    if (ast->children_num > 1 && strcmp(ast->children[1]->contents, "..") == 0) {
        generate_prec05(state, ast->children[2]);
        append_output(state,"arr.range\n");
    }
}

// Assignment operators
void generate_prec03(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec03|>", ast->tag));

    if (ast->children_num > 1) {
        state->has_lvalue++;
        if (strcmp(ast->children[1]->contents, "=") != 0) {
            if (tag_startswith(ast->children[0], "prec04")) {
                generate_prec04(state, ast->children[0]);
            } else if (tag_startswith(ast->children[0], "ident")) {
                generate_ident(state, ast->children[0]);
            }
        }

        generate_exp(state, ast->children[ast->children_num - 1]); // rhs

        if (strcmp(ast->children[1]->contents, "+=") == 0) {
            append_output(state,"add\n");
        } else if (strcmp(ast->children[1]->contents, "-=") == 0) {
            append_output(state,"sub\n");
        } else if (strcmp(ast->children[1]->contents, "**=") == 0) {
            append_output(state,"pow\n");
        } else if (strcmp(ast->children[1]->contents, "*=") == 0) {
            append_output(state,"mul\n");
        } else if (strcmp(ast->children[1]->contents, "/=") == 0) {
            append_output(state,"div\n");
        } else if (strcmp(ast->children[1]->contents, "%=") == 0) {
            append_output(state,"mod\n");
        } else if (strcmp(ast->children[1]->contents, ">>=") == 0) {
            append_output(state,"rsh\n");
        } else if (strcmp(ast->children[1]->contents, "<<=") == 0) {
            append_output(state,"lsh\n");
        } else if (strcmp(ast->children[1]->contents, "&=") == 0) {
            append_output(state,"and\n");
        } else if (strcmp(ast->children[1]->contents, "^=") == 0) {
            append_output(state,"xor\n");
        } else if (strcmp(ast->children[1]->contents, "|=") == 0) {
            append_output(state,"or\n");
        }

        state->has_lvalue--;
        state->is_lvalue++;
    }

    // lhs
    if (tag_startswith(ast->children[0], "prec04")) {
        generate_prec04(state, ast->children[0]);
    } else if (tag_startswith(ast->children[0], "ident")) {
        generate_ident(state, ast->children[0]);
    }

    if (ast->children_num > 1) {
        state->is_lvalue--;

        if (state->has_lvalue) {
            if (tag_startswith(ast->children[0], "prec04")) {
                generate_prec04(state, ast->children[0]);
            } else if (tag_startswith(ast->children[0], "ident")) {
                generate_ident(state, ast->children[0]);
            }
        }
    }
}

void generate_prec02(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec02|>", ast->tag));

    generate_prec03(state, ast->children[0]);
}

void generate_prec01(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("prec01|>", ast->tag));

    for (int i = 0; i < ast->children_num; i++) {
        if (tag_startswith(ast->children[i], "prec02")) {
            generate_prec02(state, ast->children[i]);
            if (i < ast->children_num - 1) {
                append_output(state,"pop\n");
            }
        }
    }
}

void generate_exp(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("exp|>", ast->tag));

    generate_prec01(state, ast->children[0]);
}

void generate_expstmt(generator_state_t *state, mpc_ast_t *ast) {
//    assert(0 == strcmp("stmtexp|>", ast->tag));

    generate_prec01(state, ast->children[0]);
}