//-----------------------------------------------------------------------------
//lasm2_parser.h
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#ifndef LASM2_PARSER_h
#define LASM2_PARSER_h

#include "lasm2_tokenizer.h"

typedef struct expr_node{
  token_t* token;
  struct expr_node* right;
  struct expr_node* left;
}expr_node_t;

typedef struct{
  token_t* name;
  expr_node_t* start_address;
  expr_node_t* end_address;
}branch_t;

typedef struct scope{
  branch_t* header;
  struct scope* parent;
  struct scope* childrens;
  branch_t* branches; // list
}scope_t;

typedef struct lines{
  enum line_type {EXPR, BRANCH, SCOPE} type;
  void* line;
  struct lines* next; 
}lines_t;

int parse_expression(token_t* tokens, expr_node_t* expr);
int parse_scope(token_t* tokens, scope_t* scope);
int parse_line(token_t* tokens, lines_t* lines);
void print_expression(expr_node_t* expr);
void print_scope(scope_t* scope);
void print_line(lines_t* lines);

#endif