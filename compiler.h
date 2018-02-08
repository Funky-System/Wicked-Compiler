//
// Created by Bas du Pré on 04-02-18.
//

#ifndef COMPILER_COMPILER_H
#define COMPILER_COMPILER_H

int compile_file_to_file(const char *filename, const char *output_filename, int debug);
//char* compile_file_to_string(const char *filename);
//int compile_string_to_file(const char *filename_hint, const char *input, const char *output_filename);
char* compile_string_to_string(const char *filename_hint, const char *input, int debug);
int compiler_print_parse_tree(const char *filename, int folded);

#endif //COMPILER_COMPILER_H
