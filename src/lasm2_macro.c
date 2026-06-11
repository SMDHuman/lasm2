//-----------------------------------------------------------------------------
// lasm2_macro.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_tokenreader.h"
#include "lasm2_macro.h"

#include <dirent.h>

static int get_index_if_argument(token_t* token, macro_t* macro);

//-----------------------------------------------------------------------------
int lasm_parse_macro(token_t *tokens, macro_t **macro){
  hh_darray_t *macros_array = (hh_darray_t*)malloc(sizeof(hh_darray_t));
  hh_darray_init(macros_array, sizeof(macro_t));
  if(*macro){
    for(int i = 0; ((macro_t*)(*macro))[i].content_size != 0; i++){
      hh_darray_append(macros_array, &((macro_t*)(*macro))[i]);
    }
  }
  //===================================
  token_reader_t *reader = new_token_reader(tokens);
  while(!token_reader_EOF(reader)){
    if(token_reader_peek(reader, 0)->id == MACRO_O){
      token_reader_next(reader, 1); // skip macro opener
      // skip the newline after the macro opener
      token_reader_skip_until_not(reader, NEWLINE);
      macro_t m = {0};
      //=========================================
      // Definition type
      if(token_reader_peek(reader, 0)->id == WORD){
        m.name = *token_reader_next(reader, 1);
        //==========================
        // Check if its already defined, and replace it
        for(size_t i = 0; i < hh_darray_get_item_fill(macros_array); i++){
          macro_t* macro = hh_darray_get_reference(macros_array, i);
          if(macro->name.text_size == m.name.text_size && 
            (memcmp(macro->name.text, m.name.text, macro->name.text_size) == 0)){
            hh_darray_remove_reference(macros_array, macro);
            break;
          }
        }
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
        // Make arguments local to macro
        if(m.args_size > 0){
          token_t* local_args = (token_t*)malloc(sizeof(token_t)*(m.args_size+1));
          memset(local_args, 0, sizeof(token_t)*(m.args_size+1));
          for(size_t i = 0; i < m.args_size; i++){
            local_args[i] = m.args[i*2];
          }
          m.args = local_args;
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
        m.content_size--;
        // Make content local to macro
        if(m.content_size > 0){
          token_t* local_content = (token_t*)malloc(sizeof(token_t)*(m.content_size+1));
          memset(local_content, 0, sizeof(token_t)*(m.content_size+1));
          for(size_t i = 0; i < m.content_size; i++){
            local_content[i] = m.content[i];
          }
          m.args = local_content;
        }
        //...
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
        // remove whole block if condition is not met
        else{
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
            token_reader_peek(reader, 0)->id = NONE;
            token_reader_peek(reader, 0)->text = NULL;
            token_reader_peek(reader, 0)->text_size = 0;
            token_reader_next(reader, 1);
          }
        }
      }
      //=========================================
      //Skip include type
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
  macro_t *macros = (macro_t*)malloc(sizeof(macro_t) * (hh_darray_get_item_fill(macros_array) + 1));
  for(size_t i = 0; i < hh_darray_get_item_fill(macros_array); i++){
     hh_darray_get(macros_array, i, &macros[i]);
  }
  macros[hh_darray_get_item_fill(macros_array)] = (macro_t){0}; // Null terminate the macros array
  *macro = macros;
  hh_darray_deinit(macros_array);
  free(macros_array);
  return 0;
}




//-----------------------------------------------------------------------------
int lasm_regenerate_tokens_with_macros(token_t *tokens, macro_t *macros, token_t **regenerated_tokens, lasm_file_t **include_files, char **include_paths){
  hh_darray_t *gtokens_array = (hh_darray_t*)malloc(sizeof(hh_darray_t));
  hh_darray_init(gtokens_array, sizeof(token_t));
  token_reader_t *reader = new_token_reader(tokens);
  while(!token_reader_EOF(reader)){
    //=========================================
    // Check for includes and skip macros
    if(token_reader_peek(reader, 0)->id == MACRO_O){
      token_reader_next(reader, 1);
      // Include new file
      if(token_reader_peek(reader, 0)->id == STRING_DB || token_reader_peek(reader, 0)->id == STRING_SG){
        // Append new include file to files list
        lasm_file_t* include_file; 
        if(*include_files){
          size_t size = 1; for(; include_files[size-1]->name == 0; size++);
          *include_files = realloc(*include_files, sizeof(lasm_file_t)*(size+1)); 
          include_file = include_files[size];
        }else{
          *include_files = (lasm_file_t*)malloc(sizeof(lasm_file_t)*2);
          include_file = include_files[0];
        }
        //...
        include_file->name = (char*)malloc(token_reader_peek(reader, 0)->text_size+1);
        memcpy(include_file->name, token_reader_peek(reader, 0)->text, token_reader_peek(reader, 0)->text_size);
        include_file->name[token_reader_peek(reader, 0)->text_size] = 0;
        //...
        token_t* parent_token = (token_t*)malloc(sizeof(token_t));
        memcpy(parent_token, token_reader_peek(reader, 0), sizeof(token_t));
        token_reader_next(reader, 1);
        // Find referred file in include paths
        char *name = find_file_in_paths(include_file->name, include_paths);
        free(include_file->name);
        if(!name){
          print_error_loc(parent_token);
          printf("[ERROR] Include file can not found any in paths\n");
          return -1;
        }
        include_file->name = name;
        // Tokenize the file
        printf("[INFO/INCLUDE] Tokenizing include file %s\n", include_file->name);
        token_t* new_tokens;
        int err = load_input_file(include_file->name, include_file);
        if(err) return -1;
        err = lasm_tokenizer(include_file, &new_tokens);
        if(err) return -1;
        //Dump the tokens
        for(int i = 0; new_tokens[i].id != EOT; i++){
          new_tokens[i].parent_copy = parent_token;
          hh_darray_append(gtokens_array, &new_tokens[i]);
        }
        token_reader_skip_until_not(reader, NEWLINE);
        if(!token_reader_expect(reader, MACRO_C, 0)) return -1;
        token_reader_next(reader, 1); // skip macro closer
        //print_macro(&m);
      }
      // Skip Macro
      else{
        int count_openers = 1;
        while(count_openers > 0){
          if(token_reader_peek(reader, 0)->id == MACRO_O) count_openers++;
          else if(token_reader_peek(reader, 0)->id == MACRO_C) count_openers--;
          if(token_reader_EOF(reader)){
            print_error_loc(token_reader_peek(reader, 0));
            printf("Unexpected end of file while skipping macro definition\n");
            return -1;
          }
          token_reader_next(reader, 1);
        }
      }
    }
    //=========================================
    // Check for macro usage
    uint8_t found = 0;
    if(token_reader_peek(reader, 0)->id == WORD){
      macro_t *found_macro = NULL;
      for(size_t i = 0; macros[i].name.text_size != 0; i++){
        if(macros[i].name.text_size == token_reader_peek(reader, 0)->text_size && 
          memcmp(macros[i].name.text, token_reader_peek(reader, 0)->text, macros[i].name.text_size) == 0){
          // if(token_reader_peek(reader, 0)->line > macros[i].name.line){
            found = 1;
            found_macro = &macros[i];
          // }
          break;
        }
      }
      if(found){
        size_t arg_count = token_reader_count_until(reader, COMMA, NEWLINE);
        if(token_reader_peek(reader, 1)->id != NEWLINE) arg_count += 1;
        if(token_reader_peek(reader, 1)->id == COMMA) arg_count = 0;
        // Check if the argument counts are matching
        if(found_macro->args_size != arg_count){
          print_error_loc(token_reader_peek(reader, 0));
          printf("Expected %lu arguments for macro, but %lu given\n", found_macro->args_size, arg_count);
          return -1;
        }
        token_t* parent_token = (token_t*)malloc(sizeof(token_t));
        memcpy(parent_token, token_reader_peek(reader, 0), sizeof(token_t));
        token_reader_next(reader, 1); // Skip name
        // Extract input argument if any 
        token_t* argument_tokens[arg_count]; memset(argument_tokens, 0, arg_count*sizeof(size_t));
        size_t argument_sizes[arg_count]; memset(argument_sizes, 0, arg_count*sizeof(size_t));
        for(size_t i = 0; i < arg_count; i++){
          argument_tokens[i] = token_reader_peek(reader, 0);
          argument_sizes[i] = token_reader_skip_until_either(reader, COMMA, NEWLINE);
          if(i < arg_count-1) token_reader_next(reader, 1);
        }
        // Dump the macro
        for(size_t i = 0; i < found_macro->content_size; i++){
          int if_arg = get_index_if_argument(&found_macro->content[i], found_macro);
          if(if_arg >= 0){
            for(size_t j = 0; j < argument_sizes[if_arg]; j++){
              hh_darray_append(gtokens_array, &argument_tokens[if_arg][j]);
              // ((token_t*)hh_darray_get_end_reference(gtokens_array))->line = token_reader_peek(reader, -1)->line;
              ((token_t*)hh_darray_get_end_reference(gtokens_array))->parent_copy = parent_token;
            }
          }
          else{
            hh_darray_append(gtokens_array, &found_macro->content[i]);
            // ((token_t*)hh_darray_get_end_reference(gtokens_array))->line = token_reader_peek(reader, -1)->line;
              ((token_t*)hh_darray_get_end_reference(gtokens_array))->parent_copy = parent_token;
          }
        }

        // DEBUG PRINT STUFF
        // print_single_token(&found_macro->name); printf("\n");
        // printf("  Arg count: %lu\n", arg_count);
        // for(int i = 0; i < arg_count; i++){
        //   printf("  ");
        //   for(int j = 0; j < argument_sizes[i]; j++){
        //     print_single_token(&argument_tokens[i][j]); 
        //   }
        //   printf("\n");
        // }
        // '''''''''''''''''
      }
    }
    //=========================================
    if(!found){
      if(token_reader_peek(reader, 0)->id != NONE){
        hh_darray_append(gtokens_array, token_reader_peek(reader, 0));
      }
      token_reader_next(reader, 1);
    }
    //...
    size_t array_size = hh_darray_get_item_fill(gtokens_array);
    if(array_size >= 2){
      if(((token_t*)hh_darray_get_reference(gtokens_array, array_size-1))->id == NEWLINE &&
          ((token_t*)hh_darray_get_reference(gtokens_array, array_size-2))->id == NEWLINE){
        hh_darray_popend(gtokens_array, 0);
      }
    }
  }
	// prepare output
  token_t *tokens_data;
  if(*regenerated_tokens){
    *regenerated_tokens = realloc(*regenerated_tokens, (hh_darray_get_item_fill(gtokens_array) + 1) * sizeof(token_t));
    tokens_data = *regenerated_tokens;
  }else{ 
    tokens_data = (token_t*)malloc((hh_darray_get_item_fill(gtokens_array) + 1) * sizeof(token_t));
  }
  for(size_t i = 0; i < hh_darray_get_item_fill(gtokens_array); i++){
		hh_darray_get(gtokens_array, i, &tokens_data[i]);
	}
	tokens_data[hh_darray_get_item_fill(gtokens_array)] = (token_t){.id = EOT, .text = NULL, .text_size = 0};
	*regenerated_tokens = tokens_data;
	hh_darray_deinit(gtokens_array);
  free_token_reader(reader);
	free(gtokens_array);
  return 0;
}
//-----------------------------------------------------------------------------
static int get_index_if_argument(token_t* token, macro_t* macro){
  if(token->id != WORD) return -1;
  for(size_t i = 0; i < macro->args_size; i++){
    if(macro->args[i].text_size == token->text_size && 
       memcmp(macro->args[i].text, token->text, token->text_size) == 0){
      return i;
    }
  }
  return -1;
} 

//-----------------------------------------------------------------------------
char* find_file_in_paths(char* file_name, char** paths){
  if(!file_name || !paths) return NULL;
  
  for(int i = 0; paths[i] != NULL; i++){
    DIR* dir = opendir(paths[i]);
    if(!dir) continue;
    
    struct dirent* entry;
    while((entry = readdir(dir)) != NULL){
      if(strcmp(entry->d_name, file_name) == 0){
        closedir(dir);
        // Construct full path
        size_t path_len = strlen(paths[i]);
        size_t file_len = strlen(file_name);
        char* full_path = (char*)malloc(path_len + file_len + 2); // +2 for '/' and '\0'
        sprintf(full_path, "%s/%s", paths[i], file_name);
        return full_path;
      }
    }
    closedir(dir);
  }
  return NULL;
}

//-----------------------------------------------------------------------------
void print_macro(macro_t *macro){
  printf("[MACRO]\n");
  printf("  Name: "); print_single_token(&macro->name); printf("\n");
  printf("  Arguments (%zu): ", macro->args_size);
  for(size_t i = 0; i < macro->args_size; i++){
    printf("%.*s ", macro->args[i].text_size, macro->args[i].text);
    printf(",");
  }
  printf("\n  Content (%zu tokens):\n", macro->content_size);
  for(size_t i = 0; i < macro->content_size; i++){
    printf("    ");
    print_single_token(&macro->content[i]);
    printf("\n");
  }
}