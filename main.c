#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "../optparse.h"

int main(int argc, char **argv) {
    struct optparse_long longopts[] = {
            {"print-parse-tree", 'P', OPTPARSE_NONE},
            {"output", 'o', OPTPARSE_REQUIRED},
            {"color", 'c', OPTPARSE_REQUIRED},
            {"delay", 'd', OPTPARSE_OPTIONAL},
            {0}
    };

    int option;
    struct optparse options;
    const char *output = "out.asm";

    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
            case 'P':
                break;
            case 'o':
                output = options.optarg;
                break;
            case '?':
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
        compile(filename, output);
    }

    return 0;
}
