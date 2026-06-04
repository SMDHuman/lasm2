//-----------------------------------------------------------------------------
// lasm2_macro.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_macro.h"

//-----------------------------------------------------------------------------
uint8_t lasm_parse_macro(token_t *tokens, macro_t **macro){
  hh_darray_t *macros_array = (hh_darray_t*)malloc(sizeof(hh_darray_t));
  hh_darray_init(macros_array, sizeof(macro_t));
  //===================================
  token_t *token = tokens-1;
  while(token->id != EOT){
    token++;
    if(token->id == MACRO_O){
      // skip the newline after the macro opener
      int newline_offset = 0;
      while(token[1].id == NEWLINE){token++;newline_offset++;}
      macro_t m = {0};
      //=========================================
      // Definition type
      if(token[1].id == WORD){
        m.name = token[1];
        token+=2;
      }
      //=========================================
      // If Defined types
      else if(token[1].id == QUEST || token[1].id == EXCLA){
        token_t *check_token = &token[2];
        uint8_t include = token[1].id == QUEST ? 0 : 1;
        // Check if the macro is defined and if it is gonna be included or not
        for(size_t i = 0; i < hh_darray_get_item_fill(macros_array); i++){
          macro_t* temp_macro = hh_darray_get_reference(macros_array, i);
          if(check_token->text_size == temp_macro->name.text_size && 
            memcmp(check_token->text, temp_macro->name.text, check_token->text_size) == 0){
            if(token[1].id == QUEST) include = 1;
            else if(token[1].id == EXCLA) include = 0;
            break;
          }
        }
        // If the macro is included, remove the definition part from the tokens
        if(include){
          // Remove the beginning of the macro part from the tokens
          for(int i = -newline_offset; i < 3; i++){
             token[i].id = NONE;
             token[i].text = NULL;
             token[i].text_size = 0;
          }
          // Remove the closer of the macro part from the tokens
          int32_t macro_counter = 1;
          for(size_t i = 3; token[i].id != EOT; i++){
            if(token[i].id == MACRO_O) macro_counter++;
            else if(token[i].id == MACRO_C) macro_counter--;
            if(macro_counter == 0){
              token[i].id = NONE;
              token[i].text = NULL;
              token[i].text_size = 0;
              break;
            }
          }
        }
        // If the macro is not included, remove the whole macro part from the tokens
        else{
          int32_t macro_counter = 0;
          for(size_t i = -newline_offset; token[i].id != EOT; i++){
            if(token[i].id == MACRO_O) macro_counter++;
            else if(token[i].id == MACRO_C) macro_counter--;
            // Remove the whole macro part from the tokens
            token[i].id = NONE;
            token[i].text = NULL;
            token[i].text_size = 0;
            //...
            if(macro_counter == 0){
              break;
            }
          }
        }
      }
      //=========================================
      // Include type
      else if(token[1].id == STRING_DB || token[1].id == STRING_SG){}
      // Unknown type error
      else{
        print_error_loc(token);
        printf("Unknown macro type\n");
        return -1;
      }
    }
  }
  //===================================
  return 0;
}

//-----------------------------------------------------------------------------
void print_macro(macro_t *macro){
  //TODO
}