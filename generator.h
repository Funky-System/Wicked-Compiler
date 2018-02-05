//
// Created by Bas du Pré on 23-01-18.
//

#ifndef COMPILER_GENERATOR_H
#define COMPILER_GENERATOR_H

#include <mpc/mpc.h>

#include "hashmap.h"

#define str_startswith(str, strstart) (strstr(str, strstart) == (str))

HASHMAP_FUNCS_DECLARE(symbol_table, const char, struct symbol_table_entry)

typedef struct {
    const char *filename;
    FILE *fp;
    struct hashmap symbol_table;
    char scope[2048];
    int uniqueid;
    int is_lvalue;
    int has_lvalue;
} generator_state_t;

enum symbol_type {
    SYMBOL_TYPE_LOCAL,
    SYMBOL_TYPE_GLOBAL,
    SYMBOL_TYPE_PARAM,
    SYMBOL_TYPE_FUNCTION
};

struct symbol_table_entry {
    char *name;
    int index;
    enum symbol_type type;
};

struct symbol_table_entry *get_symbol_from_ident(generator_state_t *state, const char* ident);
struct symbol_table_entry *get_symbol_from_scopedIdent(generator_state_t *state, mpc_ast_t* identtag);
void enter_scope(generator_state_t *state, const char* name);
void leave_scope(generator_state_t *state);

void generate_exp(generator_state_t *state, mpc_ast_t *ast);
void generate_expstmt(generator_state_t *state, mpc_ast_t *ast);
void generate_stmt(generator_state_t *state, mpc_ast_t *ast);
void generate_factor(generator_state_t *state, mpc_ast_t *ast);
void generate_exp(generator_state_t *state, mpc_ast_t *ast);
void generate_function(generator_state_t *state, mpc_ast_t *ast);
void generate_block(generator_state_t *state, mpc_ast_t *ast);
void generate_decl(generator_state_t *state, mpc_ast_t *ast);
void generate_if(generator_state_t *state, mpc_ast_t *ast);

void generate_while(generator_state_t *state, mpc_ast_t *ast);
void generate_do(generator_state_t* state, mpc_ast_t *ast);
void generate_for(generator_state_t *state, mpc_ast_t *ast);

void generate_arrayInit(generator_state_t *state, mpc_ast_t *ast);
void generate_arrayIndex(generator_state_t *state, mpc_ast_t *ast);

void reserve_globals(generator_state_t *state, mpc_ast_t *ast, int depth, int *num_globals);

int generate(const char* filename, const char* filename_output, mpc_ast_t *ast);
void append_output(generator_state_t *state, const char *format, ...);

#endif //COMPILER_GENERATOR_H
