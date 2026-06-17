//-----------------------------------------------------------------------------
// lasm2_assembler.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm2_assembler.h"
#include "lasm2_parser.h"
#define HH_BIGINT_IMPLEMENTATION
#include "hh_bigint.h"
#include "utils.h"

int evaluate_expression(expr_node_t* expr, hh_bigint_t* result);
int evaluate_token(token_t* token, hh_bigint_t* result);

//-----------------------------------------------------------------------------
int assemble_lines(lines_t* lines, FILE* out_file){
  lines_t* current_line = lines;
  while(1){
    // Exit conditions
    if(current_line->type == EMPTY) break;
    if(current_line == NULL) break;

    if(current_line->type == EXPR){
      hh_bigint_t* value = hh_bigint_new(0);
      int res = evaluate_expression(current_line->line, value);
      if(res){return res;}
      hh_bigint_print_hex(value);
      hh_bigint_free(value);
    }
    
    //...
    current_line = current_line->next;
  }
  return 0;
}

//-----------------------------------------------------------------------------
int evaluate_expression(expr_node_t* expr, hh_bigint_t* result){
  hh_bigint_t* right_val = NULL;
  hh_bigint_t* left_val = NULL;
  //...
  if(expr->left){
    left_val = hh_bigint_new(0);
    int res = evaluate_expression(expr->left, left_val);
    if(res){hh_bigint_free(left_val); return res;}
  }
  if(expr->right){
    right_val = hh_bigint_new(0);
    int res = evaluate_expression(expr->right, right_val);
    if(res){hh_bigint_free(right_val); return res;}
  }
  //...
  switch(expr->token->id){
    case PLUS:{
      if(left_val && right_val){
        int res = (int)hh_bigint_add(left_val, right_val, result);
        if(res){return res;}
      }
    }break;
    case MINUS:{
      if(left_val != NULL && right_val == NULL){
        hh_bigint_copy(result, left_val);
        result->sign = !result->sign;
      }
      else if(left_val && right_val){
        int res = (int)hh_bigint_subtract(left_val, right_val, result);
        if(res) return res;
      }
    }break;
    case ASTERISK:{
      if(left_val && right_val){
        int res = (int)hh_bigint_multiply(left_val, right_val, result);
        if(res){return res;}
      }
    }break;
    case SLASH:{
      if(left_val && right_val){
        int res = (int)hh_bigint_divide(left_val, right_val, result);
        if(res){return res;}
      }
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
      }
    }break;
    case EXCLA:{
      if(left_val != NULL && right_val == NULL){
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = ~result->data[i];
        }
      }
    }break;
    case BITW_AND:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] & right_val->data[i];
        }
      }
    }break;
    case BITW_OR:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] | right_val->data[i];
        }
      }
    }break;
    case BITW_XOR:{
      if(left_val && right_val){
        if(left_val->size > right_val->size) hh_bigint_resize(right_val, left_val->size);
        if(left_val->size < right_val->size) hh_bigint_resize(left_val, right_val->size);
        hh_bigint_copy(result, left_val);
        for(size_t i = 0; i < result->size; i++){
          result->data[i] = result->data[i] ^ right_val->data[i];
        }
      }
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
      }
    }break;
    case EQUAL:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_equal(left_val, right_val);
      }
    }break;
    case SMALLER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_smaller(left_val, right_val);
      }
    }break;
    case GREATER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = hh_bigint_is_bigger(left_val, right_val);
      }
    }break;
    case EQ_SMALLER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = !hh_bigint_is_bigger(left_val, right_val);
      }
    }break;
    case EQ_GREATER:{
      if(left_val && right_val){
        hh_bigint_resize(result, 1);
        result->data[0] = !hh_bigint_is_smaller(left_val, right_val);
      }
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
          int res = evaluate_expression(expr->right->right, result);
          if(res) return res;
        }else{
          int res = evaluate_expression(expr->right->left, result);
          if(res) return res;
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
      int res = evaluate_token(expr->token, result);
      if(res) return res;
    }break;
    default:{
      printf("[WARNING] Unkown Operation '%s'\n", token_id_to_string(expr->token->id));
    }break;
  }
  //...
  if(left_val) hh_bigint_free(left_val);
  if(right_val) hh_bigint_free(right_val);
  return 0;
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