#include <assert.h>
#include <src/libwickedc/mpc/mpc.h>
#include "generator.h"

void generate_while(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("stmt|>", ast->tag));
    assert(0 == strcmp(ast->children[0]->contents, "while"));
    assert(0 == strcmp("exp|>", ast->children[1]->tag));

    char startlabel[19], endlabel[19];
    sprintf(startlabel, "startwhile_%d", state->uniqueid);
    sprintf(endlabel, "endwhile_%d", state->uniqueid++);
    append_output(state,"%s:\n", startlabel);
    generate_exp(state, ast->children[1]);
    append_output(state,"brfalse %s\n", endlabel);

    generate_block(state, ast->children[3], startlabel, endlabel);
    append_output(state,"jmp %s\n%s:\n", startlabel, endlabel);
}

void generate_do(generator_state_t* state, mpc_ast_t *ast) {
    char startlabel[19], endlabel[19], checklabel[19];
    sprintf(startlabel, "@startdo_%d", state->uniqueid);
    sprintf(checklabel, "@while_%d", state->uniqueid);
    sprintf(endlabel, "@enddo_%d", state->uniqueid++);
    append_output(state,"%s:\n", startlabel);

    mpc_ast_t *doBlock = ast->children[1];
    enter_scope_with_pos(state, "^", doBlock->state.pos, checklabel, endlabel);

    for (int i = 0; i < doBlock->children_num; i++) {
        if (strcmp("stmt|>", doBlock->children[i]->tag) == 0) {
            generate_stmt(state, doBlock->children[i]);
        } else if (strcmp("block|>", doBlock->children[i]->tag) == 0) {
            //leave_scope(state, 0);
            generate_block(state, doBlock->children[i], checklabel, endlabel);
            //enter_scope_with_pos(state, "^", doBlock->state.pos, checklabel, endlabel);
        }
    }

    if (ast->children_num > 2) {
        append_output(state, "%s:\n", checklabel);
        generate_exp(state, ast->children[3]);
        append_output(state,"brtrue %s\n", startlabel);
    } else {
        append_output(state,"%s:\n", endlabel);
    }

    leave_scope(state, 0);
}

void generate_for(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("stmt|>", ast->tag));
    assert(0 == strcmp(ast->children[0]->contents, "for"));
    assert(0 == strcmp("ident|>", ast->children[1]->tag));
    assert(0 == strcmp("exp|>", ast->children[3]->tag));
    assert(0 == strcmp("block|>", ast->children[5]->tag));

    char startlabel[19], endlabel[19], inclabel[19];
    sprintf(startlabel, "startfor_%d", state->uniqueid);
    sprintf(inclabel, "continue_%d", state->uniqueid);
    sprintf(endlabel, "endfor_%d", state->uniqueid++);

    // init: i = 0
    generate_exp(state, ast->children[3]);
    append_output(state,"dup\narr.len\nld.int 0\n");

    append_output(state,"%s:\n", startlabel);
    append_output(state,"ld.stack 0\nld.stack -2\ncmp\nbge %s\n", endlabel);
    enter_scope_with_pos(state, "^", ast->children[5]->state.pos, inclabel, endlabel);
    struct symbol_table_entry *i = get_symbol_from_ident(state, ast->children[1]->contents);
    leave_scope(state, 0);
    append_output(state,"ld.stack -2\nld.stack -1\nld.arrelem\n");
    if (i->type == SYMBOL_TYPE_LOCAL) {
        append_output(state,"st.local %d\n", i->index);
    } else if (i->type == SYMBOL_TYPE_GLOBAL) {
        append_output(state,"st.ref @global_%d\n", i->index);
    }

    generate_block(state, ast->children[5], inclabel, endlabel);

    append_output(state,"%s: ld.stack 0\nld.int 1\nadd\nst.stack -1\n", inclabel);
    append_output(state,"jmp %s\n%s:\n", startlabel, endlabel);
    append_output(state,"ajs -3\n"); // cleanup
}

