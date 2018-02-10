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
                append_output(state, "link \"%s\"\ndup\n", var);
                append_output(state, "st.ref %s\n", name);
                append_output(state, "ld.mapitem \"@init\"\ncall.pop 0\n");
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
                    struct symbol_table_entry *entry = get_symbol_from_ident(state, var);
                    const char *as = var;
                    if (exportItem->children_num > 1) {
                        as = exportItem->children[2]->contents;
                    }
                    if (entry->type == SYMBOL_TYPE_GLOBAL) {
                        append_output(state, "export.as @global_%d, \"%s\"\n", entry->index, as);
                    } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
                        append_output(state, "export.as %s, \"%s\"\n", var, as);
                    } else if (entry->type == SYMBOL_TYPE_CLASS) {
                        append_output(state, "export.as %s@val, \"%s\"\n", var, as);
                    } else {
                        fprintf(stderr, "%s:%ld:%ld error: '%s' is not exportable\n", state->filename,
                                exportItem->state.row + 1,
                                exportItem->state.col, as);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }
    append_output(state, "section .text\n");
}

const char* class_get_name(mpc_ast_t *ast) {
    return ast->children[1]->contents;
}

const char* function_get_name(mpc_ast_t *ast) {
    return ast->children[1]->contents;
}

void generate_exportable_class(generator_state_t *state, mpc_ast_t *ast) {
    if (strcmp(ast->children[0]->contents, "export") == 0) {
        const char* name = class_get_name(ast->children[1]);
        append_output(state, "section .exports\n");
        append_output(state, "export.as %s@val, \"%s\"\n", name, name);
        append_output(state, "section .text\n");
        generate_class(state, ast->children[1]);
    } else {
        generate_class(state, ast->children[0]);
    }
}

void generate_exportable_function(generator_state_t *state, mpc_ast_t *ast) {
    if (strcmp(ast->children[0]->contents, "export") == 0) {
        const char* name = function_get_name(ast->children[1]);
        append_output(state, "section .exports\n");
        append_output(state, "export.as %s, \"%s\"\n", name, name);
        append_output(state, "section .text\n");
        generate_function(state, ast->children[1], "");
    } else {
        generate_function(state, ast->children[0], "");
    }
}