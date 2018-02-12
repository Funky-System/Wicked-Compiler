#include <assert.h>
#include "generator.h"

void generate_if(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("stmt|>", ast->tag));
    assert(0 == strcmp(ast->children[0]->contents, "if"));
    assert(0 == strcmp("exp|>", ast->children[1]->tag));

    generate_exp(state, ast->children[1]);

    char elselabel[19], endlabel[19];
    sprintf(elselabel, "@else_%d", state->uniqueid);
    sprintf(endlabel, "@endif_%d", state->uniqueid++);
    int elsefound = 0;
    append_output(state,"brfalse %s\n", elselabel);

    enter_scope(state, "^", NULL, NULL);

    mpc_ast_t *ifBlock = ast->children[3];
    for (int i = 0; i < ifBlock->children_num; i++) {
        if (strcmp("stmt|>", ifBlock->children[i]->tag) == 0) {
            // single line if statement
            generate_stmt(state, ifBlock->children[i]);
        } else if (strcmp("string", ifBlock->children[i]->tag) == 0 && strcmp("end", ifBlock->children[i]->contents) == 0) {
            if (!elsefound) {
                append_output(state,"%s:\n", elselabel);
            }
        } else if (strcmp("string", ifBlock->children[i]->tag) == 0 && strcmp("else", ifBlock->children[i]->contents) == 0) {
            elsefound = 1;
            append_output(state,"jmp %s\n", endlabel);
            append_output(state,"%s:\n", elselabel);
        } else if (strcmp("block|>", ifBlock->children[i]->tag) == 0) {
            leave_scope(state);
            generate_block(state, ifBlock->children[i], NULL, NULL);
            enter_scope(state, "^", NULL, NULL);
        }
    }

    if (!elsefound) {
        append_output(state,"%s:\n", elselabel);
    }
    append_output(state,"%s:\n", endlabel);

    leave_scope(state);
}