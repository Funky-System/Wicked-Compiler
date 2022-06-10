#include "mpc/mpc.h"

#include "parser.gen.h"

#include "generators/generator.h"
#include "wickedc/wickedc.h"

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
    FILE *fp;
    fp = fopen(filename_in, "r");
    if (fp == NULL) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not open file %s\n", filename_in);
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    size_t numbytes = (size_t)ftell(fp);

    // reset the file position indicator to the beginning of the file
    fseek(fp, 0L, SEEK_SET);

    char *code_in = malloc(numbytes + 1);
    if (fread(code_in, sizeof(char), numbytes, fp) != numbytes) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not read entire file %s\n", filename_in);
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    code_in[numbytes] = '\0';

    fclose(fp);

    char *output = compile_string_to_string(filename_in, code_in, debug);
    free(code_in);

    FILE *outFile = fopen(filename_out, "wb");
    if (outFile == NULL) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not write to file %s\n", filename_out);
        fprintf(stderr, "%s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    fwrite(output, sizeof(char), strlen(output), outFile);
    free(output);

    fclose(outFile);

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

    size_t len = strlen(input);
    char *input_newline = malloc(len + 2);
    memcpy(input_newline, input, len);
    input_newline[len] = '\n';
    input_newline[len + 1] = '\0';

    mpc_result_t r;
    if (mpc_parse(filename_hint, input_newline, parser_wicked, &r)) {
        mpc_unfold(r.output);
        char *output = generate(filename_hint, debug, (mpc_ast_t*)r.output);
        free(input_newline);
        mpc_ast_delete(r.output);
        return output;
    } else {
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
        free(input_newline);
        cleanup_parser_grammar();
        return 0;
    }
}

char* compile_file_to_string(const char *filename, int debug) {
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0L, SEEK_END);
    size_t numbytes = (size_t)ftell(fp);

    // reset the file position indicator to the beginning of the file
    fseek(fp, 0L, SEEK_SET);

    char *code_in = malloc(numbytes + 1);
    if (fread(code_in, sizeof(char), numbytes, fp) != numbytes) {
        int errnum = errno;
        fprintf(stderr, "Error: Could not read entire file %s\n", filename);
        fprintf(stderr, "%s", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    code_in[numbytes] = '\0';

    fclose(fp);

    char* code_out = compile_string_to_string(filename, code_in, debug);
    free(code_in);
    return code_out;
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