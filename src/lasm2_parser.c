//-----------------------------------------------------------------------------
//lasm2_parser.c
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#include "lasm2_parser.h"


int parse_expression(token_t* tokens, expr_node_t* expr);
int parse_scope(token_t* tokens, scope_t* scope);
int parse_line(token_t* tokens, lines_t* lines);
