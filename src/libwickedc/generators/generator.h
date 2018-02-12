//
// Created by Bas du Pré on 23-01-18.
//

#ifndef COMPILER_GENERATOR_H
#define COMPILER_GENERATOR_H

#include "../mpc/mpc.h"

#include "../hashmap.h"

#define str_startswith(str, strstart) (strstr(str, strstart) == (str))

HASHMAP_FUNCS_DECLARE(symbol_table, const char, struct symbol_table_entry)

typedef struct {
    int is_lvalue;
    int has_lvalue;
    int is_first_member;
    int is_last_member;
} exp_state_t;

typedef struct {
    const char *filename;
    int debug;
    char *output;
    struct hashmap symbol_table;
    char scope[2048];
    int uniqueid;
    int is_static;
    exp_state_t *exp_state;
} generator_state_t;

enum symbol_type {
    SYMBOL_TYPE_LOCAL,
    SYMBOL_TYPE_GLOBAL,
    SYMBOL_TYPE_PARAM,
    SYMBOL_TYPE_FUNCTION,
    SYMBOL_TYPE_CLASS,
    SYMBOL_TYPE_ENUM,
    SYMBOL_TYPE_FIELD,
    SYMBOL_TYPE_MODULE
};

struct symbol_table_entry {
    const char *name;
    int index;
    enum symbol_type type;
};

struct symbol_table_entry *get_symbol_from_ident(generator_state_t *state, const char* ident);
struct symbol_table_entry *get_symbol_from_scopedIdent(generator_state_t *state, mpc_ast_t* identtag);
void enter_scope(generator_state_t *state, const char* name);
void leave_scope(generator_state_t *state);

void generate_exp(generator_state_t *state, mpc_ast_t *ast);
void generate_stmt(generator_state_t *state, mpc_ast_t *ast);
void generate_factor(generator_state_t *state, mpc_ast_t *ast);
void generate_function(generator_state_t *state, mpc_ast_t *ast, const char* prefix);
void generate_block(generator_state_t *state, mpc_ast_t *ast);
void generate_decl(generator_state_t *state, mpc_ast_t *ast);
void generate_if(generator_state_t *state, mpc_ast_t *ast);
void generate_ident(generator_state_t *state, mpc_ast_t *ast);
void generate_funCall(generator_state_t *state, mpc_ast_t *ast);
void generate_methodCall(generator_state_t *state, mpc_ast_t *ast, int this_already_on_stack);

void generate_imports(generator_state_t *state, mpc_ast_t *ast);
void generate_exports(generator_state_t *state, mpc_ast_t *ast);
void generate_exportable_function(generator_state_t *state, mpc_ast_t *ast);
void generate_exportable_class(generator_state_t *state, mpc_ast_t *ast);
void generate_exportable_enum(generator_state_t *state, mpc_ast_t *ast);

void generate_while(generator_state_t *state, mpc_ast_t *ast);
void generate_do(generator_state_t* state, mpc_ast_t *ast);
void generate_for(generator_state_t *state, mpc_ast_t *ast);

void generate_arrayInit(generator_state_t *state, mpc_ast_t *ast);
void generate_arrIndex(generator_state_t *state, mpc_ast_t *ast);

void populate_symbol_table(generator_state_t *state, mpc_ast_t *ast, int depth, int *num_locals, int *num_params,
                           enum symbol_type type);
void print_symbol_table(generator_state_t *state);

void generate_class(generator_state_t *state, mpc_ast_t *ast);
void generate_new(generator_state_t *state, mpc_ast_t *ast);
void generate_prototypeof(generator_state_t *state, mpc_ast_t *ast);

void generate_enum(generator_state_t *state, mpc_ast_t *ast);

char *generate(const char* filename, int debug, mpc_ast_t *ast);
void append_output(generator_state_t *state, const char *format, ...);
void append_debug_setcontext(generator_state_t *state, mpc_ast_t *ast);
void append_debug_enterscope(generator_state_t *state, const char* prefix, const char* name);
void append_debug_leavescope(generator_state_t *state);

#endif //COMPILER_GENERATOR_H
