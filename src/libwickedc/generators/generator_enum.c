#include <assert.h>
#include "generator.h"

void generate_enum(generator_state_t *state, mpc_ast_t *ast) {
    assert(0 == strcmp("enum|>", ast->tag));
    append_debug_setcontext(state, ast);

    char *name = ast->children[1]->contents;
    append_output(state, "# Enum name: %s\n", name);

    enter_scope(state, name, NULL, NULL);

    long it = 0;

    for (int i = 3; i < ast->children_num; i++) {
        if (strcmp("enumDecl|>", ast->children[i]->tag) == 0) {
            mpc_ast_t *enumDecl = ast->children[i];
            const char* varName = enumDecl->children[0]->contents;
            if (enumDecl->children_num > 1) {
                // this decl has a iterator value
                it = strtol(enumDecl->children[2]->contents, NULL, 0);
            }
            append_output(state, "ld.int %d\n", it++);
            append_output(state, "ld.deref %s\n", name);
            append_output(state, "st.mapitem \"%s\"\n", varName);
        }
    }

    append_output(state, "\n\n");

    leave_scope(state, 0);
}


