//-----------------------------------------------------------------------------
// lasm2_tokenizer.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM2_TOKENIZER_H
#define LASM2_TOKENIZER_H

#include <stdint.h>
#include <stdio.h>

#include "hh_darray.h"

//-----------------------------------------------------------------------------
#define ERR -1

//-----------------------------------------------------------------------------
typedef enum{
	NONE,
	EOT,
	WORD,
	NUMBER,	
	STRING_DB,
	STRING_SG,
	RBRAC_O,
	RBRAC_C,
	CBRAC_O,
	CBRAC_C,
	SBRAC_O,
	SBRAC_C,
	MACRO_O,
	MACRO_C,
	MACRO_ARG,
	NEWLINE,
	HASH,
	COLON,
	PLUS,
	MINUS,
	SLASH,
	BITSHIFT_L,
	BITSHIFT_R,
	BITW_OR,
	BITW_AND,
	BITW_XOR,
	BSLASH,
	ASTERISK,
	QUEST,
	EXCLA,
	DOT,
	COMMA,
	RANGE,
	DOLLAR,
	EQUAL,
	NOTEQUAL,
	SMALLER,
	GREATER,
	EQ_SMALLER,
	EQ_GREATER,
	ASSIGN,
}TOKEN_ID;

typedef struct{
  char *name;
  char *text;
  size_t size;
  size_t line_count;
}lasm_file_t;

typedef struct t_token{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	uint32_t text_size;
	char *text;
	lasm_file_t *origin;	
	struct t_token* parent_copy;
}token_t;

//-----------------------------------------------------------------------------
int load_input_file(char* input_name, lasm_file_t* file);
uint8_t lasm_tokenizer(lasm_file_t* file, token_t** tokens);
char char_upper(char c);
char char_lower(char c);
uint8_t is_alpha(char c);
uint8_t is_alphanum(char c);
uint8_t is_inside(char c, const char* chars);
void print_warning_loc(token_t *token);
void print_error_loc(token_t *token);
void print_single_token(token_t *token);
void print_tokens_as_code(hh_darray_t* tokens);
const char* token_id_to_string(TOKEN_ID id);


#endif
