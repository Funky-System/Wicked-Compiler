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

int compile_file_to_file(const char *filename_in, const char* filename_out, int debug) {
    mpc_err_t *err = generate_parser_grammar();

    if (err != NULL) {
        mpc_err_print(err);
        mpc_err_delete(err);
        return 0;
    }

    if (filename_in != NULL) {
        mpc_result_t r;
        if (mpc_parse_contents(filename_in, parser_wicked, &r)) {
            mpc_unfold(r.output);
            char *output = generate(filename_in, debug, (mpc_ast_t*)r.output);

            FILE *fp = fopen(filename_out, "wb");

            if (fp == NULL) {
                int errnum = errno;
                printf("Error: Could not write to file %s\n", filename_out);
                printf("%s\n", strerror(errnum));
                return 0;
            }

            fputs(output, fp);

            fclose(fp);
            free(output);

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

char* compile_string_to_string(const char *filename_hint, const char *input, int debug) {
    mpc_err_t *err = generate_parser_grammar();

    if (err != NULL) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1);
        return 0;
    }

    mpc_result_t r;
    if (mpc_parse(filename_hint, input, parser_wicked, &r)) {
        mpc_unfold(r.output);
        char *output = generate(filename_hint, debug, (mpc_ast_t*)r.output);
        mpc_ast_delete(r.output);
        return output;
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
        cleanup_parser_grammar();
        return 0;
    }
}

int compiler_print_parse_tree(const char *filename, int folded) {
    mpc_err_t *err = generate_parser_grammar();

    if (err != NULL) {
        mpc_err_print(err);
        mpc_err_delete(err);
        exit(1);
        return 0;
    }

    if (filename != NULL) {
        mpc_result_t r;
        if (mpc_parse_contents(filename, parser_wicked, &r)) {
            if (!folded) {
                mpc_unfold(r.output);
            }
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
            cleanup_parser_grammar();
            return 0;
        }
    } else {
        mpc_result_t r;
        if (mpc_parse_pipe("<stdin>", stdin, parser_wicked, &r)) {
            if (!folded) {
                mpc_unfold(r.output);
            }
            mpc_ast_print(r.output);
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