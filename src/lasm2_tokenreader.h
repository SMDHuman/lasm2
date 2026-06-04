//-----------------------------------------------------------------------------
// lasm2_tokenreader.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM2_TOKENREADER_H
#define LASM2_TOKENREADER_H
#include "lasm2_tokenizer.h"

typedef struct{
	token_t *tokens;
  size_t size;
	size_t index;
}token_reader_t;

token_reader_t *new_token_reader(token_t *tokens);
void free_token_reader(token_reader_t *reader);
token_t* token_reader_peek(token_reader_t *reader, int offset);
token_t* token_reader_next(token_reader_t *reader, int offset);
token_t* token_reader_expect(token_reader_t *reader, TOKEN_ID id, int offset);
token_t* token_reader_expect_any(token_reader_t *reader, TOKEN_ID *ids, size_t ids_count, int offset);
token_t* token_reader_expect_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2, int offset);
size_t token_reader_skip_until(token_reader_t *reader, TOKEN_ID id);
size_t token_reader_skip_until_not(token_reader_t *reader, TOKEN_ID id);
size_t token_reader_skip_back_until(token_reader_t *reader, TOKEN_ID id);
size_t token_reader_skip_until_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2);
size_t token_reader_skip_back_until_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2);
size_t token_reader_get_size(token_reader_t *reader);
size_t token_reader_get_index(token_reader_t *reader);
uint8_t token_reader_EOF(token_reader_t *reader);

#endif