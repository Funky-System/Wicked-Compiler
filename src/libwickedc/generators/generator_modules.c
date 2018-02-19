#include "generator.h"

void generate_import_processor(generator_state_t *state) {
    append_output(state, "jmp @import_processor_end\n@import_processor:\n");
    append_output(state, "locals.res 3\n");
    append_output(state, "ld.reg %%r0\n");
    append_output(state, "map.getkeys\nst.local 0\n");
    append_output(state, "ld.int 0\nst.local 1\n");
    append_output(state, "ld.local 0\narr.len\nst.local 2\n");
    append_output(state, "@ip_loop:\n");
    append_output(state, "  ld.local 1\n  ld.local 2\n  lt\n");
    append_output(state, "  brfalse @ip_endloop\n");
    append_output(state, "  ld.local 0\n  ld.local 1\n");
    append_output(state, "  ld.arrelem\n");
    // top of stack is now keys[i]
    append_output(state, "  dup\n  ld.int 0\n  ld.int 1\n  str.substr\n");
    append_output(state, "  dup\n  ld.str \"@\"\n  eq\n  brfalse @not_@\n"); // if keys[i][0:1] == "@"
    append_output(state, "    jmp @ip_skip_rename\n");
    append_output(state, "  @not_@:\n");
    append_output(state, "  dup\n  ld.str \"f\"\n  eq\n  brfalse @not_f\n"); // if keys[i][0:1] == "f"
    append_output(state, "    # do nothing, keep ref\n");
    append_output(state, "  @not_f:\n");
    append_output(state, "  dup\n  ld.str \"c\"\n  eq\n  brfalse @not_c\n"); // if keys[i][0:1] == "c"
    append_output(state, "    ld.reg %%r0\n    ld.stack -2\n    ld.mapitem.pop\n");
    append_output(state, "    deref\n");
    append_output(state, "    ld.reg %%r0\n    ld.stack -3\n    st.mapitem.pop\n");
    append_output(state, "  @not_c:\n");
    append_output(state, "  dup\n  ld.str \"e\"\n  eq\n  brfalse @not_e\n"); // if keys[i][0:1] == "e"
    append_output(state, "    ld.reg %%r0\n    ld.stack -2\n    ld.mapitem.pop\n");
    append_output(state, "    deref\n");
    append_output(state, "    ld.reg %%r0\n    ld.stack -3\n    st.mapitem.pop\n");
    append_output(state, "  @not_e:\n");
    append_output(state, "  pop\n"); // pop keys[i][0:1]
    append_output(state, "  ld.reg %%r0\n");
    append_output(state, "  ld.stack -1\n  dup\n  ld.int 1\n  ld.int -1\n  str.substr\n");
    append_output(state, "  map.renamekey.pop\n");
    append_output(state, "  @ip_skip_rename: pop\n");
    append_output(state, "  ld.local 1\n  ld.int 1\n  add\n  st.local 1\n"); // i++
    append_output(state, "  jmp @ip_loop\n");

    append_output(state, "@ip_endloop:\n");
    append_output(state, "ld.reg %%r0\ndel.mapitem \"@init\"\n");
    append_output(state, "locals.cleanup\n");
    append_output(state, "ret\n");
    append_output(state, "@import_processor_end:\n\n");
}

void generate_imports(generator_state_t *state, mpc_ast_t *ast) {
    int import_processor_outputted = 0;

    for (int i = 0; i < ast->children_num; i++) {
        if (strcmp(ast->children[i]->tag, "import|>") == 0) {
            if (!import_processor_outputted) {
                generate_import_processor(state);
                import_processor_outputted = 1;
            }

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
                append_output(state, "dup\nst.reg %%r0\n");
                append_output(state, "has.mapitem \"@init\"\n");
                append_output(state, "brfalse @skip_init_%s\n", name);
                append_output(state, "ld.reg %%r0\n");
                append_output(state, "ld.mapitem \"@init\"\ncall.pop 0\n");
                append_output(state, "call @import_processor, 0\n");
                append_output(state, "@skip_init_%s:\n", name);
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
                        append_output(state, "export.as @global_%d, \"v%s\"\n", entry->index, as);
                    } else if (entry->type == SYMBOL_TYPE_FUNCTION) {
                        append_output(state, "export.as %s, \"f%s\"\n", var, as);
                    } else if (entry->type == SYMBOL_TYPE_CLASS) {
                        append_output(state, "export.as %s, \"c%s\"\n", var, as);
                    } else if (entry->type == SYMBOL_TYPE_ENUM) {
                        append_output(state, "export.as %s, \"e%s\"\n", var, as);
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
    if (strcmp(ast->children[0]->contents, "syscall") == 0) {
        if (strcmp(ast->children[1]->tag, "syscallAlias|>") == 0) {
            return ast->children[2]->contents;
        }
    }
    return ast->children[1]->contents;
}

const char* enum_get_name(mpc_ast_t *ast) {
    return ast->children[1]->contents;
}

void generate_exportable_class(generator_state_t *state, mpc_ast_t *ast) {
    if (strcmp(ast->children[0]->contents, "export") == 0) {
        const char* name = class_get_name(ast->children[1]);
        append_output(state, "section .exports\n");
        append_output(state, "export.as %s, \"c%s\"\n", name, name);
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
        append_output(state, "export.as %s, \"f%s\"\n", name, name);
        append_output(state, "section .text\n");
        generate_function(state, ast->children[1], "");
    } else {
        generate_function(state, ast->children[0], "");
    }
}

void generate_exportable_enum(generator_state_t *state, mpc_ast_t *ast) {
    if (strcmp(ast->children[0]->contents, "export") == 0) {
        const char* name = enum_get_name(ast->children[1]);
        append_output(state, "section .exports\n");
        append_output(state, "export.as %s, \"e%s\"\n", name, name);
        append_output(state, "section .text\n");
        generate_enum(state, ast->children[1]);
    } else {
        generate_enum(state, ast->children[0]);
    }
}