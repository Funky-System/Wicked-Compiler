#import "generator.h"

void generate_imports(generator_state_t *state, mpc_ast_t *ast) {
    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "import|>") == 0) {
            mpc_ast_t *import = ast->children[i];
            const char *var = import->children[1]->contents;
            const char *as = var;
            if (strcmp(import->children[2]->contents, "as") == 0) {
                as = import->children[3]->contents;
            }

            if (symbol_table_hashmap_get(&state->symbol_table, as) == NULL) {
                struct symbol_table_entry *e = malloc(sizeof(struct symbol_table_entry));
                char *name = malloc(32);
                sprintf(name, "@import_%d", state->uniqueid++);
                e->name = name;
                e->type = SYMBOL_TYPE_MODULE;
                symbol_table_hashmap_put(&state->symbol_table, as, e);
                append_output(state,"section .data\n%s: var\nsection .text\n", name);
                append_output(state, "link \"%s\"\n", var);
                append_output(state, "st.addr %s\n", name);
                // TODO: name is never freed
            } else {
                fprintf(stderr, "%s:%ld:%ld error: '%s' is already defined\n", state->filename,
                        import->state.row + 1,
                        import->state.col, as);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void generate_exports(generator_state_t *state, mpc_ast_t *ast) {
    append_output(state, "section .exports\n");
    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "export|>") == 0) {
            mpc_ast_t *export = ast->children[i];
            for (int j = 0; j < export->children_num; j++) {
                if (strcmp(export->children[j]->tag, "exportItem|>") == 0) {
                    mpc_ast_t *exportItem = export->children[j];
                    const char *var = exportItem->children[0]->contents;
                    get_symbol_from_ident(state, var); // to force namechecking
                    const char *as = var;
                    if (exportItem->children_num > 1) {
                        as = exportItem->children[2]->contents;
                    }
                    append_output(state, "export.as %s, \"%s\"\n", var, as);
                }
            }
        }
    }
    append_output(state, "section .text\n");
}
