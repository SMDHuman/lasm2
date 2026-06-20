//-----------------------------------------------------------------------------
// lasm2_assembler.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM2_ASSEMBLER_H
#define LASM2_ASSEMBLER_H

#include "lasm2_parser.h"
#include "hh_bigint.h"

typedef struct assembly_scope{
  branch_t* header;
  struct assembly_scope* parent_scope;
  struct assembly_scope** sub_scopes;
  size_t sub_scopes_size;
}assembly_scope_t;

typedef struct{
  size_t index;
  size_t size;
  expr_node_t* expr;
}assembly_patch_t;

typedef struct{
  uint8_t branch_default_size;
  FILE* out_file;
}assembly_config_t;

typedef struct assembly{
  assembly_config_t* config;
  lines_t* root_line;
  assembly_scope_t* scopes;
  assembly_patch_t* patches;
}assembly_t;

assembly_t* lasm2_assembly_new(lines_t* lines, assembly_config_t* config);
assembly_scope_t* lasm2_assembly_extract_scope_tree(lines_t* lines);
int lasm2_assemble(assembly_t *assembly);
int lasm2_assembly_free(assembly_t* assembly);
int lasm2_evaluate_expression(assembly_t *assembly, expr_node_t* expr, hh_bigint_t* result);
int evaluate_token(assembly_t* assembler, token_t* token, hh_bigint_t* result);

void print_assembly_scope(assembly_scope_t* assembly_scope);
void print_assembly_scope_indent(assembly_scope_t* assembly_scope, int indent);


#endif