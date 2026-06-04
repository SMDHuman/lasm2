//-----------------------------------------------------------------------------
// lasm2_macro.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM2_MACRO_H
#define LASM2_MACRO_H

#include <stdint.h>
#include "lasm2_tokenizer.h"
//-----------------------------------------------------------------------------

typedef struct{
  token_t name;
  token_t* args; // <xxx <arg1>, <arg2>, ...>
  token_t* content; // <...>
  size_t args_size;
  size_t content_size;
}macro_t;

//-----------------------------------------------------------------------------
int lasm_parse_macro(token_t *tokens, macro_t **macro);
void print_macro(macro_t *macro);

#endif