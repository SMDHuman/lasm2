//-----------------------------------------------------------------------------
// lasm2_tokenreader.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------

#include "lasm2_tokenreader.h"

//-----------------------------------------------------------------------------
token_reader_t *new_token_reader(token_t *tokens){
  token_reader_t *reader = malloc(sizeof(token_reader_t));
  reader->tokens = tokens;
  reader->size = 0;
  while(tokens[reader->size].id != EOT){
    reader->size++;
  }
  reader->size+= 1;
  reader->index = 0;
  return reader;
}
//-----------------------------------------------------------------------------
void free_token_reader(token_reader_t *reader){
  free(reader);
}
//-----------------------------------------------------------------------------
token_t* token_reader_peek(token_reader_t *reader, int offset){
  if((long)reader->index + offset >= (long)reader->size){
    return &reader->tokens[reader->size-1];
  }
  else if((long)reader->index + offset < 0) return &reader->tokens[0];
  return &reader->tokens[reader->index + offset];
}
//-----------------------------------------------------------------------------
token_t* token_reader_next(token_reader_t *reader, int offset){
  token_t *token = token_reader_peek(reader, 0);
  if(token == NULL) return NULL;
  if((long)reader->index + offset >= (long)reader->size) reader->index = reader->size;
  else if((long)reader->index + offset < 0) reader->index = 0;
  else reader->index += offset;

  return token;
}
//-----------------------------------------------------------------------------
token_t* token_reader_expect(token_reader_t *reader, TOKEN_ID id, int offset){
  token_t *token = token_reader_peek(reader, offset);
  if(token->id != id){
    print_error_loc(token);
    printf("Expected token of type %s but got %s\n", token_id_to_string(id), token_id_to_string(token->id));
    return NULL;
  }
  return token;
}
//-----------------------------------------------------------------------------
token_t* token_reader_expect_any(token_reader_t *reader, TOKEN_ID *ids, size_t ids_count, int offset){
  token_t *token = token_reader_peek(reader, offset);
  for(size_t i = 0; i < ids_count; i++){
    if(token->id == ids[i]){
      return token;
    }
  }
  print_error_loc(token);
  printf("Expected token of type ");
  for(size_t i = 0; i < ids_count; i++){
    printf("%s ", token_id_to_string(ids[i]));
  }
  printf("but got %s\n", token_id_to_string(token->id));
  return NULL;
}
//-----------------------------------------------------------------------------
token_t* token_reader_expect_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2, int offset){
  token_t *token = token_reader_peek(reader, offset);
  if(token->id == id1 || token->id == id2){
    return token;
  }
  print_error_loc(token);
  printf("Expected token of type %s or %s but got %s\n", token_id_to_string(id1), token_id_to_string(id2), token_id_to_string(token->id));
  return NULL;
}
//-----------------------------------------------------------------------------
size_t token_reader_skip_until(token_reader_t *reader, TOKEN_ID id){
  size_t skipped = 0;
  while(token_reader_peek(reader, 0)->id != id){
    if(token_reader_peek(reader, 0)->id == EOT) return 0;
    token_reader_next(reader, 1);
    skipped++;
  }
  return skipped;
}
//-----------------------------------------------------------------------------
size_t token_reader_skip_until_not(token_reader_t *reader, TOKEN_ID id){
  size_t skipped = 0;
  while(token_reader_peek(reader, 0)->id == id){
    if(token_reader_peek(reader, 0)->id == EOT) return 0;
    token_reader_next(reader, 1);
    skipped++;
  }
  return skipped;
}
//-----------------------------------------------------------------------------
size_t token_reader_skip_back_until(token_reader_t *reader, TOKEN_ID id){
  size_t skipped = 0;
  while(token_reader_peek(reader, -1 - skipped)->id != id){
    if(token_reader_peek(reader, -1 - skipped)->id == EOT) return 0;
    skipped++;
  }
  reader->index -= skipped;
  return skipped;
}
//-----------------------------------------------------------------------------
size_t token_reader_skip_until_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2){
  size_t skipped = 0;
  while(token_reader_peek(reader, skipped)->id != id1 && token_reader_peek(reader, skipped)->id != id2){
    if(token_reader_peek(reader, skipped)->id == EOT) return 0;
    skipped++;
  }
  reader->index += skipped;
  return skipped;
}
//-----------------------------------------------------------------------------
size_t token_reader_skip_back_until_either(token_reader_t *reader, TOKEN_ID id1, TOKEN_ID id2){
  size_t skipped = 0;
  while(token_reader_peek(reader, -1 - skipped)->id != id1 && token_reader_peek(reader, -1 - skipped)->id != id2){
    if(token_reader_peek(reader, -1 - skipped)->id == EOT) return 0;
    skipped++;
  }
  reader->index -= skipped;
  return skipped;
}
//-----------------------------------------------------------------------------
size_t token_reader_count_until(token_reader_t *reader, TOKEN_ID count, TOKEN_ID until){
  size_t counted = 0;
  for(size_t i = 0; token_reader_peek(reader, i)->id != until && token_reader_peek(reader, i)->id != EOT; i++){
    if(token_reader_peek(reader, i)->id == count) counted++;
  }
  return counted;
}
//-----------------------------------------------------------------------------
size_t token_reader_get_size(token_reader_t *reader){
  return reader->size;
}
//-----------------------------------------------------------------------------
size_t token_reader_get_index(token_reader_t *reader){
  return reader->index;
}
//-----------------------------------------------------------------------------
uint8_t token_reader_EOF(token_reader_t *reader){
  return reader->index >= reader->size;
}
