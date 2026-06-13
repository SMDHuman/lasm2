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
}branch_t;

typedef struct lines{
  enum line_type {EMPTY, EXPR, SCOPE} type;
  void* line;
  struct lines* next; 
}lines_t;

typedef struct scope{
  branch_t* header;
  struct scope* parent;
  lines_t* lines;
}scope_t;

static int parse_expression(token_reader_t* reader, expr_node_t* root_expr);
static int parse_scope(token_reader_t* reader, scope_t* scope);
static int parse_line(token_reader_t* reader, lines_t* lines, scope_t* parent);
int lasm2_parser(token_t* tokens, lines_t* lines);
void print_expression(expr_node_t* expr, int indent);
void print_scope(scope_t* scope, int indent);
void print_line(lines_t* lines, int indent);

#endif