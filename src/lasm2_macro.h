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
  token_t* names;
  size_t name_count;
  token_t* args; // <xxx <arg1>, <arg2>, ...>
  token_t* content; // <...>
  size_t args_size;
  size_t content_size;
}macro_t;

//-----------------------------------------------------------------------------

int lasm_parse_macro(token_t *tokens, macro_t **macro);
int lasm_regenerate_tokens_with_macros(token_t *tokens, macro_t *macros, token_t **regenerated_tokens, lasm_file_t **include_files, char **include_paths);
char* find_file_in_paths(char* file_name, char** paths);
void print_macro(macro_t *macro);
void free_macro(macro_t* macro);
void free_macros(macro_t* macros);

#endif