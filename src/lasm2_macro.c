//-----------------------------------------------------------------------------
// lasm2_macro.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_tokenreader.h"
#include "lasm2_macro.h"

//-----------------------------------------------------------------------------
int lasm_parse_macro(token_t *tokens, macro_t **macro){
  hh_darray_t *macros_array = (hh_darray_t*)malloc(sizeof(hh_darray_t));
  hh_darray_init(macros_array, sizeof(macro_t));
  //===================================
  token_reader_t *reader = new_token_reader(tokens);
  while(!token_reader_EOF(reader)){
    if(token_reader_peek(reader, 0)->id == MACRO_O){
      token_reader_next(reader, 1); // skip macro opener
      // skip the newline after the macro opener
      int newline_offset = token_reader_skip_until_not(reader, NEWLINE);
      macro_t m = {0};
      //=========================================
      // Definition type
      if(token_reader_peek(reader, 0)->id == WORD){
        m.name = *token_reader_next(reader, 1);
        //==========================
        // Parse arguments
        if(!token_reader_expect_either(reader, WORD, NEWLINE, 0)) return -1;
        if(token_reader_peek(reader, 0)->id == WORD){
          m.args = token_reader_peek(reader, 0);
          m.args_size = 1;
          token_reader_next(reader, 1);
          while(token_reader_peek(reader, 0)->id != NEWLINE){
            if(!token_reader_expect(reader, COMMA, 0)) return -3;
            if(!token_reader_expect(reader, WORD, 1)) return -4;
            token_reader_next(reader, 2);
            m.args_size++;
          }
        }
        token_reader_next(reader, 1); // skip newline
        //==========================
        // Parse content
        m.content = token_reader_peek(reader, 0);
        m.content_size = 0;
        int count_openers = 1;
        while(count_openers > 0){
          if(token_reader_peek(reader, 0)->id == MACRO_O) count_openers++;
          else if(token_reader_peek(reader, 0)->id == MACRO_C) count_openers--;
          if(token_reader_EOF(reader)){
            print_error_loc(&m.name);
            printf("Unexpected end of file while parsing macro content\n");
            return -1;
          }
          token_reader_next(reader, 1);
          m.content_size++;
        }
        //print_macro(&m);
        hh_darray_append(macros_array, &m);
      }
      //=========================================
      // If Defined types
      else if(token_reader_peek(reader, 0)->id == QUEST || token_reader_peek(reader, 0)->id == EXCLA){
        uint8_t is_ifdef = token_reader_next(reader, 1)->id == QUEST;
        token_reader_skip_until_not(reader, NEWLINE);
        if(!token_reader_expect(reader, WORD, 0)) return -2;
        m.name = *token_reader_next(reader, 1);
        if(!token_reader_expect(reader, NEWLINE, 0)) return -1;
        token_reader_next(reader, 1); // skip newline
        //==========================
        // Check if macro is defined or not
        uint8_t found = 0;
        for(size_t i = 0; i < hh_darray_get_item_fill(macros_array); i++){
          macro_t *defined_macro = hh_darray_get_reference(macros_array, i);
          if(defined_macro->name.text_size == m.name.text_size && 
            memcmp(defined_macro->name.text, m.name.text, m.name.text_size) == 0){
            found = 1;
            break;
          }
        }
        // remove the macro closer and tags to extract only the content
        if(found == is_ifdef){
          token_t *token = token_reader_peek(reader, -1);
          while(token->id != MACRO_O){
            token->id = NONE;
            token->text = NULL;
            token->text_size = 0;
            token--;
          }
          token->id = NONE;
          token->text = NULL;
          token->text_size = 0;

          int count_openers = 1;
          while(count_openers > 0){
            if(token_reader_peek(reader, 0)->id == MACRO_O) count_openers++;
            else if(token_reader_peek(reader, 0)->id == MACRO_C) count_openers--;
            if(token_reader_EOF(reader)){
              print_error_loc(&m.name);
              printf("Unexpected end of file while parsing macro content\n");
              return -1;
            }
            token_reader_next(reader, 1);
          }
          token_reader_peek(reader, -1)->id = NONE;
          token_reader_peek(reader, -1)->text = NULL;
          token_reader_peek(reader, -1)->text_size = 0;
        }
      }
      //=========================================
      // Include type
      else if(token_reader_peek(reader, 0)->id == STRING_DB || token_reader_peek(reader, 0)->id == STRING_SG){
        m.name = *token_reader_next(reader, 1);
        token_reader_skip_until_not(reader, NEWLINE);
        if(!token_reader_expect(reader, MACRO_C, 0)) return -1;
        token_reader_next(reader, 1); // skip macro closer
        //print_macro(&m);
      }
      // Unknown type error
      else{
        print_error_loc(token_reader_peek(reader, 0));
        print_single_token(token_reader_peek(reader, 0));
        printf("Unknown macro type\n");
        printf("\n");
        return -1;
      }
    }else{
      token_reader_next(reader, 1);
    }
    // Detect unexpected macro closer
    if(token_reader_peek(reader, 0)->id == MACRO_C){
      print_error_loc(token_reader_peek(reader, 0));
      printf("Unexpected macro closer\n");
      return -1;
    }
  }
  //===================================
  return 0;
}

//-----------------------------------------------------------------------------
void print_macro(macro_t *macro){
  printf("[MACRO]\n");
  printf("  Name: "); print_single_token(&macro->name); printf("\n");
  printf("  Arguments (%zu): ", macro->args_size);
  for(size_t i = 0; i < macro->args_size; i++){
    printf("%.*s ", macro->args[i*2].text_size, macro->args[i*2].text);
    printf(",");
  }
  printf("\n  Content (%zu tokens):\n", macro->content_size);
  for(size_t i = 0; i < macro->content_size; i++){
    printf("    ");
    print_single_token(&macro->content[i]);
    printf("\n");
  }
}