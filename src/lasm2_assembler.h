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
  lines_t* root_line;
  struct assembly_scope* parent_scope;
  struct assembly_scope** sub_scopes;
  size_t sub_scopes_size;
  assign_t** assignments;
  size_t assignments_size;
}assembly_scope_t;

typedef struct assembly_patch{
  size_t index;
  size_t size;
  expr_node_t* expr;
  assembly_scope_t* scope;
  struct assembly_patch* next;
}assembly_patch_t;

typedef struct{
  uint8_t branch_default_size;
  FILE* out_file;
}assembly_config_t;

typedef struct assembly{
  assembly_config_t* config;
  assembly_scope_t* root_scope;
  assembly_scope_t* current_scope;
  assembly_patch_t* root_patch;
  assembly_patch_t* leaf_patch;
}assembly_t;

assembly_t* lasm2_assembly_new(lines_t* lines, assembly_config_t* config);
int lasm2_assembly_free(assembly_t* assembly);
void lasm2_assembly_scope_free(assembly_scope_t* scope);
int lasm2_assemble(assembly_t *assembly);
int lasm2_assemble_patches(assembly_t *assembly);
int lasm2_evaluate_expression(assembly_t *assembly, expr_node_t* expr, hh_bigint_t* result);
int evaluate_token(token_t* token, hh_bigint_t* result);
branch_t* find_header_with_token_in_scopes(assembly_scope_t* assembly_scope, token_t* token);
assign_t* find_assignment_with_token_in_scopes(assembly_scope_t* assembly_scope, token_t* token);

void print_assembly_scope(assembly_scope_t* assembly_scope);
void print_assembly_scope_indent(assembly_scope_t* assembly_scope, int indent);


#endif