#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

int main(int argc, char **argv) {
    struct optparse_long longopts[] = {
            {"print-parse-tree", 'P', OPTPARSE_OPTIONAL},
            {"simulate", 'S', OPTPARSE_NONE},
            {"output", 'o', OPTPARSE_REQUIRED},
            {"do-not-assemble", 'A', OPTPARSE_NONE},
            {"assembler", '.', OPTPARSE_REQUIRED},
            {"keep-asm", 'K', OPTPARSE_NONE},
            {0}
    };

    int option;
    struct optparse options;

    const char *output = "out.funk";
    char *asm_output = malloc(strlen(output) + 5);
    strcpy(asm_output, output);
    strcat(asm_output, ".fasm");
    int print_parse_tree = 0, print_parse_tree_folded = 1;
    int assemble = 1;
    char *assembler = "funky-as";
    int keep_asm = 0;

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
            case 'P':
                print_parse_tree = 1;
                if (options.optarg == NULL) break;

                if (strcmp("unfold", options.optarg) == 0) {
                    print_parse_tree_folded = 0;
                } else if (strcmp("fold", options.optarg) == 0) {
                    print_parse_tree_folded = 1;
                } else {
                    fprintf(stderr, "%s: Unsupported argument to --print-parse-tree: '%s'\n", argv[0], options.optarg);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                output = options.optarg;
                asm_output = malloc(strlen(output) + 5);
                strcpy(asm_output, output);
                strcat(asm_output, ".fasm");
                break;
            case 'S':
                output = NULL;
                assemble = 0;
                break;
            case 'A': // do-not-assemble:
                assemble = 0;
                break;
            case 'K':
                keep_asm = 1;
                break;
            case '.': // assembler:
                assembler = options.optarg;
                break;
            case '?':
            default:
                fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
                exit(EXIT_FAILURE);
        }
    }

    if (options.optind >= argc) {
        fprintf(stderr, "%s: error: no input files\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *filename;
    while ((filename = optparse_arg(&options))) {
        if (print_parse_tree) {
            compiler_print_parse_tree(filename, print_parse_tree_folded);
        }

        if (!assemble) {
            compile_file_to_file(filename, output);
        } else  {
            if (compile_file_to_file(filename, asm_output)) {
                char *command = malloc(strlen(assembler) + strlen(asm_output) + strlen(output) + 16);
                strcpy(command, assembler);
                strcat(command, " \"");
                strcat(command, asm_output);
                strcat(command, "\"");
                strcat(command, " --output \"");
                strcat(command, output);
                strcat(command, "\"");
                int ret = system(command);
                if (!keep_asm) remove(asm_output);
                if (ret == 0) {
                    // yay!
                    return 0;
                } else {
                    return ret;
                }
            }
        }
    }

    return 0;
}
