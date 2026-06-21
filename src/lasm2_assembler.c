//-----------------------------------------------------------------------------
// lasm2_assembler.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#define HH_BIGINT_IMPLEMENTATION
#include "hh_bigint.h"
#include "lasm2_assembler.h"
#include "lasm2_parser.h"
#define LASM2_UTILS_IMPLEMENTATION
#include "utils.h"

//-----------------------------------------------------------------------------
assembly_t* lasm2_assembly_new(lines_t* lines, assembly_config_t* config){
  assembly_t* new = NEW(assembly_t, 1);
  new->config = NEW(assembly_config_t, 1); 
  memcpy(new->config, config, sizeof(assembly_config_t));
  new->root_scope = lasm2_assembly_extract_scope_tree(lines);
  new->root_scope->header = NULL;
  new->root_scope->parent_scope = NULL;
  new->current_scope = new->root_scope;
  print_assembly_scope(new->root_scope);
  return new;
}

//-----------------------------------------------------------------------------
void lasm2_assembly_scope_free(assembly_scope_t* scope){
  if(scope == NULL) return;
  
  // Recursively free all sub-scopes
  for(size_t i = 0; i < scope->sub_scopes_size; i++){
    lasm2_assembly_scope_free(scope->sub_scopes[i]);
  }
  
  // Free the sub_scopes array
  if(scope->sub_scopes != NULL){
    free(scope->sub_scopes);
  }
  
  // Free the scope itself
  free(scope);
}

//-----------------------------------------------------------------------------
int lasm2_assembly_free(assembly_t* assembly){
  if(assembly == NULL) return 0;
  
  // Free the config
  if(assembly->config != NULL){
    free(assembly->config);
  }
  
  // Free the scope tree
  if(assembly->root_scope != NULL){
    lasm2_assembly_scope_free(assembly->root_scope);
  }
  
  // Free patches if allocated
  if(assembly->patches != NULL){
    free(assembly->patches);
  }
  
  // Free the assembly itself
  free(assembly);
  
  return 0;
}
//-----------------------------------------------------------------------------
int lasm2_assemble(assembly_t *assembly){
  //...
  lines_t* current_line = assembly->current_scope->root_line;
  //...
  while(!(current_line == NULL || current_line->type == EMPTY)){
    if(current_line->type == EXPR){
      hh_bigint_t* value = hh_bigint_new(0);
      int res = lasm2_evaluate_expression(assembly, current_line->line, value);
      fwrite(value->data, 1, value->size, assembly->config->out_file);
      hh_bigint_print_hex(value);
      hh_bigint_free(value);
      if(res) return res;
    }
    if(current_line->type == SCOPE){
      //...
      scope_t* scope = current_line->line;
      assembly_scope_t* found_asm_scope = NULL;
      for(int i = 0; i < assembly->current_scope->sub_scopes_size; i++){
        assembly_scope_t* asm_scope = assembly->current_scope->sub_scopes[i];
        if(asm_scope->header == scope->header){
          found_asm_scope = asm_scope;
          // printf("YEaHHH\n");
        }
      }
      //...
      if(found_asm_scope){
        // Evaluate scope
        if(found_asm_scope->header->start_address){
          hh_bigint_t* address_gint = hh_bigint_new(0);
          int res = lasm2_evaluate_expression(assembly, found_asm_scope->header->start_address, address_gint);
          if(res){
            print_warning_loc(found_asm_scope->header->name);
            printf("Branch position declaration should not include further branches");
            return res;
          }
          found_asm_scope->header->location = hh_bigint_get_uint64(address_gint);
          found_asm_scope->header->eval_flag = 1;
          hh_bigint_free(address_gint);
        }else{
          found_asm_scope->header->location = ftell(assembly->config->out_file);
          found_asm_scope->header->eval_flag = 1;
        }
        //...
        assembly->current_scope = found_asm_scope;
        int res = lasm2_assemble(assembly);
        if(res) return res;
        assembly->current_scope = assembly->current_scope->parent_scope;
      }
    }
    current_line = current_line->next;
  }
  return 0;
}

//-----------------------------------------------------------------------------
assembly_scope_t* lasm2_assembly_extract_scope_tree(lines_t* lines){
  assembly_scope_t* assembly_scope = NEW(assembly_scope_t, 1);
  assembly_scope->root_line = lines;
  hh_darray_t sub_scopes_array; hh_darray_init(&sub_scopes_array, sizeof(assembly_scope_t*));
  lines_t* current_line = lines;
  while(!(current_line == NULL || current_line->type == EMPTY)){
    //...
    if(current_line->type == SCOPE){
      scope_t* scope = current_line->line;
      assembly_scope_t* sub_scope = lasm2_assembly_extract_scope_tree(scope->lines);
      sub_scope->header = scope->header;
      sub_scope->parent_scope = assembly_scope;
      hh_darray_append(&sub_scopes_array, &sub_scope);
    }
    current_line = current_line->next;
  }
  // Deinitialize array 
  assembly_scope->sub_scopes_size = hh_darray_get_item_fill(&sub_scopes_array);
  if(assembly_scope->sub_scopes_size > 0){
    assembly_scope->sub_scopes = NEW(assembly_scope_t*, assembly_scope->sub_scopes_size);
  }else assembly_scope->sub_scopes = NULL;
  for(int i = 0; i < assembly_scope->sub_scopes_size; i++){
    hh_darray_get(&sub_scopes_array, i, &assembly_scope->sub_scopes[i]);
  }
  hh_darray_deinit(&sub_scopes_array);
  return assembly_scope;
}

//-----------------------------------------------------------------------------
int lasm2_evaluate_expression(assembly_t *assembly, expr_node_t* expr, hh_bigint_t* result){
  int err = 0;
  if(expr->token == NULL){
    printf("[ERROR] No valid token found\n");
    return -1;
  }
  hh_bigint_t* right_val = NULL;
  hh_bigint_t* left_val = NULL;
  //...
  if(expr->left){
    left_val = hh_bigint_new(0);
    err = lasm2_evaluate_expression(assembly, expr->left, left_val);
  }
  if(expr->right){
    right_val = hh_bigint_new(0);
    err = lasm2_evaluate_expression(assembly, expr->right, right_val);
  }
  //...
  if(!err){
    switch(expr->token->id){
      case PLUS:{
        if(left_val && right_val){
          if(hh_bigint_add(left_val, right_val, result)){err = -1;break;}
        }else{err = -1; break; }
      }break;
      case MINUS:{
        if(left_val != NULL && right_val == NULL){
          hh_bigint_copy(result, left_val);
          result->sign = !result->sign;
        }
        else if(left_val && right_val){
          if(hh_bigint_subtract(left_val, right_val, result)){err = -1;break;}
        }else{ err = -1; break; }
      }break;
      case ASTERISK:{
        if(left_val && right_val){
          if(hh_bigint_multiply(left_val, right_val, result)){err = -1;break;}
        }else{ err = -1; break; }
      }break;
      case SLASH:{
        if(left_val && right_val){
          if(hh_bigint_divide(left_val, right_val, result)){err = -1;break;}
        }else{ err = -1; break; }
      }break;
      case SBRAC_O:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          uint64_t index = hh_bigint_get_uint64(right_val);
          if(index < left_val->size){
            result->data[0] = left_val->data[index];
          }else{
            result->data[0] = 0;
          }
        }else{ err = -1; break; }
      }break;
      case BITSHIFT_L:{
        if(left_val && right_val){
          uint64_t amount = hh_bigint_get_uint64(right_val);
          if(hh_bigint_shift_left(left_val, amount, result)){err = -1;break;}
        }else{ err = -1; break; }
      }break;
      case BITSHIFT_R:{
        if(left_val && right_val){
          uint64_t amount = hh_bigint_get_uint64(right_val);
          if(hh_bigint_shift_right(left_val, amount, result)){err = -1;break;}
        }else{ err = -1; break; }
      }break;
      case EXCLA:{
        if(left_val != NULL && right_val == NULL){
          hh_bigint_copy(result, left_val);
          for(size_t i = 0; i < result->size; i++){
            result->data[i] = ~result->data[i];
          }
        }else{ err = -1; break; }
      }break;
      case BITW_AND:{
        if(left_val && right_val){
          if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
          if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
          hh_bigint_copy(result, left_val);
          for(size_t i = 0; i < result->size; i++){
            result->data[i] = result->data[i] & right_val->data[i];
          }
        }else{ err = -1; break; }
      }break;
      case BITW_OR:{
        if(left_val && right_val){
          if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
          if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
          hh_bigint_copy(result, left_val);
          for(size_t i = 0; i < result->size; i++){
            result->data[i] = result->data[i] | right_val->data[i];
          }
        }else{ err = -1; break; }
      }break;
      case BITW_XOR:{
        if(left_val && right_val){
          if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
          if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
          hh_bigint_copy(result, left_val);
          for(size_t i = 0; i < result->size; i++){
            result->data[i] = result->data[i] ^ right_val->data[i];
          }
        }else{ err = -1; break; }
      }break;
      case DOLLAR:{
        if(left_val != NULL && right_val == NULL){
          uint64_t size = left_val->size;
          hh_bigint_set_uint64(result, size);
          hh_bigint_normalize(result);
        }
        else if(left_val && right_val){
          uint64_t size = hh_bigint_get_uint64(right_val);
          hh_bigint_copy(result, left_val);
          int res = (int)hh_bigint_resize(result, size);
          if(res) return res;
        }else{ err = -1; break; }
      }break;
      case EQUAL:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          result->data[0] = hh_bigint_is_equal(left_val, right_val);
        }else{ err = -1; break; }
      }break;
      case SMALLER:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          result->data[0] = hh_bigint_is_smaller(left_val, right_val);
        }else{ err = -1; break; }
      }break;
      case GREATER:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          result->data[0] = hh_bigint_is_bigger(left_val, right_val);
        }else{ err = -1; break; }
      }break;
      case EQ_SMALLER:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          result->data[0] = !hh_bigint_is_bigger(left_val, right_val);
        }else{ err = -1; break; }
      }break;
      case EQ_GREATER:{
        if(left_val && right_val){
          hh_bigint_resize(result, 1);
          result->data[0] = !hh_bigint_is_smaller(left_val, right_val);
        }else{ err = -1; break; }
      }break;
      case COLON:{
        if(left_val==NULL || right_val==NULL){
          print_error_loc(expr->token);
          printf("Expecting expressions between colon ':'\n");
        }
      }break;
      case QUEST:{
        if(expr->right->token->id == COLON){
          hh_bigint_t* zero = hh_bigint_new(0);
          if(hh_bigint_is_equal(left_val, zero)){
            err = lasm2_evaluate_expression(assembly, expr->right->right, result);
            if(err) break;
          }else{
            err = lasm2_evaluate_expression(assembly, expr->right->left, result);
            if(err) break;
          }
        }else{
          print_error_loc(expr->token);
          printf("Expecting colon ':' after the selector.\n");
        }
      }break;
      case STRING_DB:
      case STRING_SG:
      case NUMBER:{
        if(left_val || right_val){
          print_error_loc(expr->token);
          printf("Something is wrong with the expression.\n");
          err = -1; break;
        }
        if(evaluate_token(assembly, expr->token, result)){err = -1; break;}
      }break;
      case WORD:{
        branch_t* branch = find_header_with_token_in_scopes(assembly->current_scope, expr->token);
        if(branch->eval_flag){
          hh_bigint_set_uint64(result, branch->location);
          hh_bigint_resize(result, assembly->config->branch_default_size);
        }else{
          // TODO
        }
      }break;
      default:{
        print_warning_loc(expr->token);
        printf("Unkown Operation '%s'\n", token_id_to_string(expr->token->id));
      }break;
    }
  }
  //...
  if(left_val) hh_bigint_free(left_val);
  if(right_val) hh_bigint_free(right_val);
  if(err == -1){
    print_error_loc(expr->token);
    printf("Something unexpected happen while evaluating this expression.\n");
  }else if(err < 0){
    return err-1;
  }
  return err;
}
//-----------------------------------------------------------------------------
branch_t* find_header_with_token_in_scopes(assembly_scope_t* assembly_scope, token_t* token){
  //...
  char* name = NEW(char, token->text_size+1);
  char* leading_name = NULL;
  memcpy(name, token->text, token->text_size);
  if(str_chr_count(name, '.')){
    leading_name = str_first_chr(name, '.') + 1;
    *(leading_name-1) = '\0';
  }
  //...
  branch_t* found_header = NULL;
  for(int i = 0; i < assembly_scope->sub_scopes_size; i++){
    assembly_scope_t* asm_scope = assembly_scope->sub_scopes[i];
    if(strlen(name) == asm_scope->header->name->text_size && 
       memcmp(name, asm_scope->header->name->text, asm_scope->header->name->text_size) == 0){
      if(leading_name){
        token_t* temp_token = NEW(token_t, 1);
        memcpy(temp_token, token, sizeof(token_t));
        temp_token->text = leading_name;
        temp_token->text_size = strlen(leading_name);
        found_header = find_header_with_token_in_scopes(asm_scope, temp_token);
        free(temp_token);
      }else{
        found_header = asm_scope->header;
      }
      break;
    }
  }
  //...
  if(found_header == NULL && assembly_scope->parent_scope){
    found_header = find_header_with_token_in_scopes(assembly_scope->parent_scope, token);
  }
  free(name);
  return found_header;
}

//-----------------------------------------------------------------------------
void print_assembly_scope(assembly_scope_t* assembly_scope){
  if(assembly_scope == NULL){
    printf("NULL assembly_scope\n");
    return;
  }
  print_assembly_scope_indent(assembly_scope, 0);
}

//-----------------------------------------------------------------------------
void print_assembly_scope_indent(assembly_scope_t* assembly_scope, int indent){
  if(assembly_scope == NULL){
    return;
  }
  
  for(int i = 0; i < indent; i++) printf("  ");
  printf("AssemblyScope:");
  
  if(assembly_scope->header != NULL){
    printf("name=");
    if(assembly_scope->header->name != NULL){
      print_single_token(assembly_scope->header->name);
    }
    printf(", location=%lu, eval_flag=%u", assembly_scope->header->location, assembly_scope->header->eval_flag);
  } else {
    printf("(root scope)");
  }
  printf("\n");
  
  // Print all sub-scopes
  for(int i = 0; i < assembly_scope->sub_scopes_size; i++){
    print_assembly_scope_indent(assembly_scope->sub_scopes[i], indent + 1);
  }
}

//-----------------------------------------------------------------------------
int evaluate_token(assembly_t* assembler, token_t* token, hh_bigint_t* result){
  if(token->id == NUMBER){
    char* text = NEW(char, token->text_size+1);
    memcpy(text, token->text, token->text_size); text[token->text_size] = 0;
    if(hh_bigint_convert_from_string(result, text)){
      print_error_loc(token);
      printf("Illegal number.\n");
    }
    free(text);
  }
  else if(token->id == STRING_DB || token->id == STRING_SG){
    hh_bigint_resize(result, token->text_size);
    memcpy(result->data, token->text, token->text_size);
  }
  return 0;
}
