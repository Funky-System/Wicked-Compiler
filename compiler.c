#include <mpc/mpc.h>
#include "mpc/mpc.h"

#include "parser.gen.h"

#include "generator.h"
#include "compiler.h"

void mpc_unfold(mpc_ast_t *ast) {
    char *loc = strchr(ast->tag, '|');
    if (loc != NULL && *(loc + 1) != '>') {
        // unfold this
        char *tag = malloc(strlen(ast->tag));
        strcpy (tag, loc + 1);
        mpc_ast_t *sub = mpc_ast_new(tag, ast->contents);
        free(tag);
        sub = mpc_ast_state(sub, ast->state);

        for (int i = 0; i < ast->children_num; i++) {
            mpc_ast_add_child(sub, ast->children[i]);
        }

        ast->children_num = 0;
        ast = mpc_ast_add_child(ast, sub);

        loc[1] = '>';
        loc[2] = '\0';
        mpc_unfold(sub);
    }

    for (int i = 0; i < ast->children_num; i++) {
        mpc_unfold(ast->children[i]);
    }
}

int compile(const char *filename, const char *output_filename) {
    mpc_err_t *err = generate_parser_grammar();

    if (err != NULL) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1);
        return 0;
    }

    if (filename != NULL) {
        mpc_result_t r;
        if (mpc_parse_contents(filename, parser_sourcecode, &r)) {
            mpc_unfold(r.output);
            //mpc_ast_print(r.output);
            generate(filename, output_filename, (mpc_ast_t*)r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            cleanup_parser_grammar();
            return 0;
        }
    } else {
        mpc_result_t r;
        if (mpc_parse_pipe("<stdin>", stdin, parser_sourcecode, &r)) {
            mpc_ast_print(r.output);
            generate("<stdin>", output_filename, (mpc_ast_t*)r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            cleanup_parser_grammar();
            return 0;
        }
    }

    return 1;
}
