//-----------------------------------------------------------------------------
//lasm2_parser.h
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#ifndef LASM2_PARSER_h
#define LASM2_PARSER_h

#include "lasm2_tokenizer.h"
#include "lasm2_tokenreader.h"

typedef struct expr_node{
  token_t* token;
  struct expr_node* right;
  struct expr_node* left;
}expr_node_t;

typedef struct{
  token_t* name;
  expr_node_t* start_address;
  expr_node_t* end_address;
  uint64_t location;
  uint8_t eval_flag; // 0 if location is not evaluated
}branch_t;

typedef struct lines{
  enum line_type {EMPTY, EXPR, SCOPE, ASSIGNMENT} type;
  void* line;
  struct lines* next; 
}lines_t;

typedef struct scope{
  branch_t* header;
  expr_node_t* condition;
  enum scope_type {SCOPE_NORMAL, SCOPE_IF, SCOPE_ELSE} type;
  struct scope* parent;
  lines_t* lines;
}scope_t;

typedef struct{
  token_t* name;
  expr_node_t* value;
}assign_t;


int lasm2_parser(token_t* tokens, lines_t* lines);
void print_expression(expr_node_t* expr, int indent);
void print_scope(scope_t* scope, int indent);
void print_line(lines_t* lines, int indent);

void free_expr_node(expr_node_t* expr);
void free_branch(branch_t* branch);
void free_scope(scope_t* scope);
void free_lines(lines_t* lines);

#endif