#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "wickedc/wickedc.h"

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"
#include "version.h"

int main(int argc, char **argv) {
    struct optparse_long longopts[] = {
            {"print-parse-tree", 'P', OPTPARSE_OPTIONAL},
            {"simulate", 'S', OPTPARSE_NONE},
            {"output", 'o', OPTPARSE_REQUIRED},
            {"do-not-assemble", 'A', OPTPARSE_NONE},
            {"assembler", '.', OPTPARSE_REQUIRED},
            {"keep-asm", 'K', OPTPARSE_NONE},
            {"only-if-newer", 'N', OPTPARSE_NONE},
            {"debug", 'd', OPTPARSE_OPTIONAL},
            {"verbose", 'V', OPTPARSE_NONE},
            {"version", 'v', OPTPARSE_NONE},
            {"arch", 'a', OPTPARSE_REQUIRED},
            {0}
    };

    int option;
    struct optparse options;

    const char *output = "out.funk";
    char *asm_output = malloc(strlen(output) + 6);
    strcpy(asm_output, output);
    strcat(asm_output, ".fasm");
    int print_parse_tree = 0, print_parse_tree_folded = 1;
    int assemble = 1;
    char *assembler = "funky-as";
    int keep_asm = 0;
    int verbose = 0;
    int only_if_newer = 0;
    char* arch = "32";
    int debug = 1;

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
                asm_output = malloc(strlen(output) + 6);
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
            case 'N': // only-if-newer
                only_if_newer = 1;
                break;
            case 'a':
                arch = options.optarg;
                break;
            case 'K':
                keep_asm = 1;
                break;
            case '.': // assembler:
                assembler = options.optarg;
                break;
            case 'd': // debug
                debug = options.optarg ? (int)strtol(options.optarg, NULL, 0) : 1;
                break;
            case 'V': // verbose
                verbose = 1;
                break;
            case 'v':
                printf("Funky Wicked Compiler version %s.%s.%s\nBuilt on %s %s\n", VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, __DATE__, __TIME__);
                return 0;
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
        if (only_if_newer) {
            struct stat filestat_in, filestat_out;
            int res_in = stat(output, &filestat_out);
            int res_out = stat(filename, &filestat_in);
            if (res_in == 0 && res_out == 0 && filestat_in.st_mtime <= filestat_out.st_mtime) {
                if (verbose) {
                    fprintf(stderr, "Skipping %s because output file %s is newer\n", filename, output);
                }
                continue;
            }
        }

        if (print_parse_tree) {
            compiler_print_parse_tree(filename, print_parse_tree_folded);
        }

        if (!assemble) {
            compile_file_to_file(filename, output, debug);
        } else  {
            if (compile_file_to_file(filename, asm_output, debug)) {
                char *command = malloc(strlen(assembler) + strlen(asm_output) + strlen(output) + 1024);
                sprintf(command, "%s \"%s\" --arch %s --output \"%s\"", assembler, asm_output, arch, output);
                //printf("command: %s\n", command);
                int ret = system(command);

                free(asm_output);
                free(command);

                if (ret == 0) {
                    if (!keep_asm) 
                        remove(asm_output);
                    // yay!
                    return 0;
                } else {
                    return 1;
                }
            }
        }
    }

    free(asm_output);

    return 0;
}
