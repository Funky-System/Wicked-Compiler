#include <assert.h>
#include "generator.h"

void generate_stmt(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("stmt|>", ast->tag));

    if (strcmp("return", ast->children[0]->contents) == 0) {
        if (ast->children_num > 1 && strcmp("exp|>", ast->children[1]->tag) == 0) {
            generate_exp(state, ast->children[1]);
            append_output(state,"st.reg %%rr\n");
        }
        append_output(state,"locals.cleanup\n");
        append_output(state,"args.cleanup\n");
        append_output(state,"ret\n");
    } else if (strcmp(ast->children[0]->tag, "asm|>") == 0) {
        char code[strlen(ast->children[0]->children[1]->contents) + 1];
        strcpy(code, ast->children[0]->children[1]->contents);
        code[strlen(code) - 1] = '\0';
        append_output(state,"%s\n", code + 1);
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
    }

}

void generate_block(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("block|>", ast->tag));

    enter_scope(state, "^");

    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp("stmt|>", ast->children[i]->tag) == 0) {
            generate_stmt(state, ast->children[i]);
        } else if (strcmp("string", ast->children[i]->tag) == 0 && strcmp("end", ast->children[i]->contents) == 0) {
            append_output(state,"# end\n");
        }
    }

    leave_scope(state);
}