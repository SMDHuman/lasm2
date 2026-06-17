//-----------------------------------------------------------------------------
// lasm2_assembler.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm2_assembler.h"
#include "lasm2_parser.h"
#define HH_BIGINT_IMPLEMENTATION
#include "hh_bigint.h"
#include "utils.h"

typedef enum{
  EVAL_OK,
  EVAL_ERROR,
  EVAL_UNKOWN_LOC,
  EVAL_UNKOWN_BRANCH,
  EVAL_UNKOWN_SCOPE,
  EVAL_UNKOWN_OP,
  EVAL_ERROR_TOKEN,
  EVAL_INSUFFICIENT_EXPR
}expr_eval_e;

typedef struct assembler{
  lines_t* root_line;
  FILE* out_file;
  scope_t** shallow_scopes;
  struct assembler* parent;
}assembler_t;

expr_eval_e evaluate_expression(expr_node_t* expr, hh_bigint_t* result, assembler_t* assembler);
int evaluate_token(token_t* token, hh_bigint_t* result);
int assemble(assembler_t* self);

//-----------------------------------------------------------------------------
int lasm2_assemble_to_file(lines_t* lines, FILE* out_file){
  assembler_t main = {0};
  main.root_line = lines;
  main.out_file = out_file;
  int res = assemble(&main);
  if(res) return res;
  return 0;
}

//-----------------------------------------------------------------------------
int assemble(assembler_t* self){
  //...
  hh_darray_t scope_array; hh_darray_init(&scope_array, sizeof(scope_t*));
  for(lines_t* current_line = self->root_line; current_line->type != EMPTY; current_line = current_line->next){
    if(current_line->type == SCOPE && ((scope_t*)current_line->line)->header != NULL){
      hh_darray_append(&scope_array, &current_line->line);
    }
  }
  self->shallow_scopes = NEW(scope_t*, hh_darray_get_item_fill(&scope_array)+1);
  self->shallow_scopes[hh_darray_get_item_fill(&scope_array)] = NULL;
  for(size_t i = 0; i < hh_darray_get_item_fill(&scope_array); i++){
    hh_darray_get(&scope_array, i, &self->shallow_scopes[i]);
  }
  hh_darray_deinit(&scope_array);
  //...
  lines_t* current_line = self->root_line;
  while(1){
    // Exit conditions
    if(current_line->type == EMPTY) break;
    if(current_line == NULL) break;

    if(current_line->type == EXPR){
      hh_bigint_t* value = hh_bigint_new(0);
      expr_eval_e res = evaluate_expression(current_line->line, value, self);
      if(res == EVAL_ERROR){return res;}
      fwrite(value->data, 1, value->size, self->out_file);
      hh_bigint_print_hex(value);
      hh_bigint_free(value);
    }
    
    //...
    current_line = current_line->next;
  }
  return 0;
}

//-----------------------------------------------------------------------------
expr_eval_e evaluate_expression(expr_node_t* expr, hh_bigint_t* result, assembler_t* assembler){
  expr_eval_e err = EVAL_OK;
  hh_bigint_t* right_val = NULL;
  hh_bigint_t* left_val = NULL;
  //...
  if(expr->left){
    left_val = hh_bigint_new(0);
    expr_eval_e res = evaluate_expression(expr->left, left_val, assembler);
    if(res){hh_bigint_free(left_val); return res;}
  }
  if(expr->right){
    right_val = hh_bigint_new(0);
    expr_eval_e res = evaluate_expression(expr->right, right_val, assembler);
    if(res){hh_bigint_free(right_val); return res;}
  }
  //...
  switch(expr->token->id){
    case PLUS:{
      if(left_val && right_val){
        if(hh_bigint_add(left_val, right_val, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case MINUS:{
      if(left_val != NULL && right_val == NULL){
        hh_bigint_copy(result, left_val);
        result->sign = !result->sign;
      }
      else if(left_val && right_val){
        if(hh_bigint_subtract(left_val, right_val, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case ASTERISK:{
      if(left_val && right_val){
        if(hh_bigint_multiply(left_val, right_val, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case SLASH:{
      if(left_val && right_val){
        if(hh_bigint_divide(left_val, right_val, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
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
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case BITSHIFT_L:{
      if(left_val && right_val){
        uint64_t amount = hh_bigint_get_uint64(right_val);
        if(hh_bigint_shift_left(left_val, amount, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case BITSHIFT_R:{
      if(left_val && right_val){
        uint64_t amount = hh_bigint_get_uint64(right_val);
        if(hh_bigint_shift_right(left_val, amount, result)){err = EVAL_ERROR;break;}
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case EXCLA:{
      if(left_val != NULL && right_val == NULL){
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = ~result->data[i];
        }
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case BITW_AND:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] & right_val->data[i];
        }
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case BITW_OR:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] | right_val->data[i];
        }
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case BITW_XOR:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] ^ right_val->data[i];
        }
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
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
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case EQUAL:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_equal(left_val, right_val);
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case SMALLER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_smaller(left_val, right_val);
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case GREATER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_bigger(left_val, right_val);
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case EQ_SMALLER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = !hh_bigint_is_bigger(left_val, right_val);
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
    }break;
    case EQ_GREATER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = !hh_bigint_is_smaller(left_val, right_val);
      }else{ err = EVAL_INSUFFICIENT_EXPR; break; }
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
          err = evaluate_expression(expr->right->right, result, assembler);
          if(err) break;
        }else{
          err = evaluate_expression(expr->right->left, result, assembler);
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
      }
      if(evaluate_token(expr->token, result)){err = EVAL_ERROR_TOKEN; break;}
    }break;
    case WORD:{
      if(expr->token->text_size == 1 && expr->token->text[0] == '_'){
        hh_bigint_resize(result, 0);
      }
      else{
        print_warning_loc(expr->token);
        printf("Unkown branch name.\n");
        err = EVAL_UNKOWN_BRANCH;
      }
    }break;
    default:{
      print_warning_loc(expr->token);
      printf("Unkown Operation '%s'\n", token_id_to_string(expr->token->id));
    }break;
  }
  //...
  if(left_val) hh_bigint_free(left_val);
  if(right_val) hh_bigint_free(right_val);
  return err;
}

//-----------------------------------------------------------------------------
int evaluate_token(token_t* token, hh_bigint_t* result){
  if(token->id == NUMBER){
    char* text = NEW(char, token->text_size+1);
    memcpy(text, token->text, token->text_size); text[token->text_size] = 0;
    if(hh_bigint_convert_from_string(result, text)){
      print_error_loc(token);
      printf("Illegal number.\n");
    }
    free(text);
  }
  if(token->id == STRING_DB || token->id == STRING_SG){
    hh_bigint_resize(result, token->text_size);
    memcpy(result->data, token->text, token->text_size);
  }
  return 0;
}