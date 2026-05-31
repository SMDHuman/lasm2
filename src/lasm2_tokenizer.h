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
}TOKEN_ID;

typedef struct{
  char *name;
  char *text;
  size_t size;
  size_t line_count;
}lasm_file_t;

typedef struct{
	uint16_t id;
	uint32_t line;
	uint32_t col;
	lasm_file_t *origin;	
	char *text;
	uint32_t text_size;
}token_t;

uint8_t lasm_tokenizer(lasm_file_t* file, char *filename, token_t** tokens);
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
