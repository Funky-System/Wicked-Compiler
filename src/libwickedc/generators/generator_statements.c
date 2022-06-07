#include <assert.h>
#include <src/libwickedc/mpc/mpc.h>
#include "generator.h"
#include "src/libwickedc/string_functions.h"


void generate_return(generator_state_t *state, const mpc_ast_t *ast);
void generate_continue(generator_state_t *state, mpc_ast_t *ast);
void generate_break(generator_state_t *state, mpc_ast_t *ast);

void generate_asm(generator_state_t *state, const mpc_ast_t *ast);

void generate_stmt(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("stmt|>", ast->tag));

    append_debug_setcontext(state, ast);

    if (strcmp("return", ast->children[0]->contents) == 0) {
        generate_return(state, ast);
    } else if (strcmp(ast->children[0]->tag, "asm|>") == 0) {
        generate_asm(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "continue") == 0) {
        generate_continue(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "break") == 0) {
        generate_break(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "if") == 0) {
        generate_if(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "while") == 0) {
        generate_while(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "for") == 0) {
        generate_for(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "do") == 0) {
        generate_do(state, ast);
    } else if (strcmp(ast->children[0]->tag, "string") == 0 && strcmp(ast->children[0]->contents, "var") == 0) {
        for (int i = 1; i < ast->children_num; i++) {
            if (0 == strcmp("decl|>", ast->children[i]->tag)) {
                generate_decl(state, ast->children[i]);
            }
        }
    } else if (strcmp(ast->children[0]->tag, "expstmt|>") == 0) {
        generate_exp(state, ast->children[0]);
        // ignore the result
        append_output(state, "pop\n");
    }

}

void generate_asm(generator_state_t *state, const mpc_ast_t *ast) {
    size_t len = strlen(ast->children[0]->children[1]->contents) + 1;
    char* code = malloc(len);
    strcpy(code, ast->children[0]->children[1]->contents);
    code[strlen(code) - 1] = '\0';
    char* verbatim = str_replace(code + 1, "\\\"", "\"");
    free(code);

    append_output(state,"%s\n", verbatim);
    free(verbatim);
}

void generate_return(generator_state_t *state, const mpc_ast_t *ast) {
    if (state->is_conv_method) {
        if (ast->children_num > 1 && strcmp("exp|>", ast->children[1]->tag) == 0) {
            generate_exp(state, ast->children[1]);
        } else {
            append_output(state, "ld.empty\n");
        }
        append_output(state, "ld.local 1\n");
        append_output(state, "ld.int 1\nsub\n");
        append_output(state, "st.arg.pop\n");
        append_output(state, "locals.cleanup\n");
        append_output(state, "args.cleanup\n");
        append_debug_leavescope(state);
        append_output(state, "ret\n");
    } else {
        if (ast->children_num > 1 && strcmp("exp|>", ast->children[1]->tag) == 0) {
            generate_exp(state, ast->children[1]);
            append_output(state, "st.reg %%rr\n");
        }
        append_output(state, "locals.cleanup\n");
        append_output(state, "args.cleanup\n");
        append_debug_leavescope(state);
        append_output(state, "ret\n");
    }
}

void generate_block(generator_state_t *state, mpc_ast_t *ast, const char* continue_label, const char* break_label) {
    assert(0 == strcmp("block|>", ast->tag));

    char endLabel[32];
    sprintf(endLabel, "@end_%d", state->uniqueid++);

    enter_scope_with_pos(state, "^", ast->state.pos, continue_label, break_label);

    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp("stmt|>", ast->children[i]->tag) == 0) {
            generate_stmt(state, ast->children[i]);
        } else if (strcmp("string", ast->children[i]->tag) == 0 && strcmp("end", ast->children[i]->contents) == 0) {
            append_output(state,"%s: # end\n", endLabel);
        }
    }

    leave_scope(state, 0);
}

void generate_continue(generator_state_t *state, mpc_ast_t *ast) {
    if (state->num_continue_labels == 0) return;
    int i = (int)state->num_continue_labels - 1;
    char* label = state->continue_labels[i];
    while (label == NULL && i > 0) {
        i--;
        label = state->continue_labels[i];
    }
    if (label != NULL) {
        append_output(state, "jmp %s\n", label);
    }
}

void generate_break(generator_state_t *state, mpc_ast_t *ast) {
    if (state->num_break_labels == 0) return;
    int i = (int)state->num_break_labels - 1;
    char* label = state->break_labels[i];
    while (label == NULL && i > 0) {
        i--;
        label = state->break_labels[i];
    }
    if (label != NULL) {
        append_output(state, "jmp %s\n", label);
    }
}