//-----------------------------------------------------------------------------
//lasm2_parser.c
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#include "lasm2_parser.h"
#include "lasm2_tokenizer.h"
#include "lasm2_tokenreader.h"
#include "utils.h"

static int token_prc(token_t *token);
static expr_node_t* parse_expression_right(token_reader_t* reader, int min_prc);

//-----------------------------------------------------------------------------
int parse_expression(token_reader_t* reader, expr_node_t* root_expr){
  int prev_precedence = 0;
  
  expr_node_t* buffer_expr = parse_expression_right(reader, 1);
  memcpy(root_expr, buffer_expr, sizeof(expr_node_t));
  while(1){
    if(token_reader_EOF(reader)) break;
    if(token_reader_peek(reader, 0)->id == NEWLINE) break;
    if(token_reader_peek(reader, 0)->id == SBRAC_C) break;
    if(token_reader_peek(reader, 0)->id == RBRAC_C) break;
    if(token_prc(token_reader_peek(reader, 0)) <= 0) break;

    buffer_expr = parse_expression_right(reader, 1);
    buffer_expr->left = NEW(expr_node_t, 1);
    memcpy(buffer_expr->left, root_expr, sizeof(expr_node_t));
    memcpy(root_expr, buffer_expr, sizeof(expr_node_t));

  }
  // print_single_token(token_reader_peek(reader, 0));printf("\n");

  return 0;
}

static expr_node_t* parse_expression_right(token_reader_t* reader, int min_prc){
  if(token_reader_EOF(reader)) return NULL;
  if(token_reader_peek(reader, 0)->id == NEWLINE) return NULL;
  expr_node_t* new_expr = NEW(expr_node_t, 1);
  new_expr->left = 0; 
  new_expr->right = 0; 
  new_expr->token = 0;
  if(token_reader_peek(reader, 0)->id == NUMBER || 
     token_reader_peek(reader, 0)->id == STRING_DB || 
     token_reader_peek(reader, 0)->id == STRING_SG || 
     token_reader_peek(reader, 0)->id == WORD){
    new_expr->token = token_reader_peek(reader, 0);
    token_reader_next(reader, 1);
  }
  if(token_reader_peek(reader, 0)->id == RBRAC_O){
    token_reader_next(reader, 1);
    parse_expression(reader, new_expr);
    return new_expr;
  }
  
  if(token_reader_peek(reader, 0)->id == SBRAC_O){
    token_t* index_token = token_reader_next(reader, 1);
    expr_node_t* new_op_expr= NEW(expr_node_t, 1);
    new_op_expr->left = new_expr;
    new_op_expr->token = index_token;
    new_op_expr->right = NEW(expr_node_t, 1);
    parse_expression(reader, new_op_expr->right);
    return new_op_expr;
  }

  // EXIT ON ']' ')'
  if(token_reader_peek(reader, 0)->id == RBRAC_C || token_reader_peek(reader, 0)->id == SBRAC_C) {
    token_reader_next(reader, 1);
    return new_expr;
  }
  // print_single_token(token_reader_peek(reader, 0));printf("\n");
  // printf("prc, min_prc: %d, %d\n", token_prc(token_reader_peek(reader, 0)),  min_prc);
  if(token_prc(token_reader_peek(reader, 0)) < min_prc){
    return new_expr;
  }else{
    expr_node_t* new_op_expr= NEW(expr_node_t, 1);
    new_op_expr->left = new_expr;
    new_op_expr->token = token_reader_next(reader, 1);
    new_op_expr->right = parse_expression_right(reader, token_prc(new_op_expr->token));
    return new_op_expr;
  }
}

//-----------------------------------------------------------------------------
int parse_scope(token_t* tokens, scope_t* scope){
  return 0;
}

int parse_line(token_t* tokens, lines_t* lines){
  token_reader_t* reader = new_token_reader(tokens);
  lines_t* root_line = lines;
  lines_t* current_line = root_line;
  while(!token_reader_EOF(reader)){
    // Parse as scope
    if(token_reader_peek(reader, 0)->id == DOT || token_reader_peek(reader, 0)->id == CBRAC_O){
      token_reader_next(reader, 1);
    }
    else if(token_reader_peek(reader, 0)->id == NEWLINE){
      token_reader_next(reader, 1);
    }
    // Parse as expression
    else{
      expr_node_t* expr = NEW(expr_node_t, 1);
      int res = parse_expression(reader, expr);
      if(res) return res;
      current_line->type = EXPR;
      current_line->line = expr;
      current_line->next = NEW(lines_t, 1);
      current_line = current_line->next;
      current_line->type = EMPTY;
      current_line->next = 0;
      current_line->line = 0;
    }
  }
}

//-----------------------------------------------------------------------------
static int token_prc(token_t *token){
  int precedence = 1;
  if(token->id == SMALLER || token->id == EQ_SMALLER) return precedence;
  if(token->id == GREATER || token->id == EQ_GREATER) return precedence;
  if(token->id == EQUAL || token->id == NOTEQUAL) return precedence;
  precedence++;
  if(token->id == BITW_OR) return precedence;
  if(token->id == BITW_XOR) return precedence;
  if(token->id == BITW_AND) return precedence;
  precedence++;
  if(token->id == BITSHIFT_L || token->id == BITSHIFT_R) return precedence;
  precedence++;
  if(token->id == PLUS || token->id == MINUS) return precedence;
  precedence++;
  if(token->id == ASTERISK || token->id == SLASH) return precedence;
  precedence++;
  if(token->id == DOLLAR || token->id == EXCLA) return precedence;
  precedence++;
  return 0; // Default precedence for other tokens
}
//-----------------------------------------------------------------------------
void print_expression(expr_node_t* expr){
  if(expr == NULL){
    printf("NULL");
    return;
  }
  
  
  if(expr->left && expr->right) printf("(");
  if(expr->token != NULL){
    // Bunu yap
    char text[expr->token->text_size + 1];
    memcpy(text, expr->token->text, expr->token->text_size);
    text[expr->token->text_size] = 0;
    printf(" %s ", text);
  }
  if(expr->left != NULL){
    print_expression(expr->left);
  }
  
  if(expr->right != NULL){
    print_expression(expr->right);
  }
  if(expr->left && expr->right) printf(")");
}
//-----------------------------------------------------------------------------
void print_scope(scope_t* scope){
  if(scope == NULL){
    printf("NULL scope\n");
    return;
  }
  
  printf("Scope: ");
  if(scope->header != NULL){
    printf("name=");
    if(scope->header->name != NULL){
      print_single_token(scope->header->name);
    }
    printf(" start=");
    print_expression(scope->header->start_address);
    printf(" end=");
    print_expression(scope->header->end_address);
  }
  printf("\n");
  
  if(scope->branches != NULL){
    printf("  Branches:\n");
    branch_t* current_branch = scope->branches;
    while(current_branch != NULL){
      printf("    Branch: name=");
      if(current_branch->name != NULL){
        print_single_token(current_branch->name);
      }
      printf(" start=");
      print_expression(current_branch->start_address);
      printf(" end=");
      print_expression(current_branch->end_address);
      printf("\n");
      current_branch = current_branch + 1; // Assuming array-like structure
    }
  }
  
  if(scope->childrens != NULL){
    printf("  Child scopes:\n");
    scope_t* current_child = scope->childrens;
    while(current_child != NULL){
      printf("    ");
      print_scope(current_child);
      current_child = current_child->childrens; // Navigate to next sibling
    }
  }
}
//-----------------------------------------------------------------------------
void print_line(lines_t* lines){
  if(lines == NULL){
    printf("NULL line\n");
    return;
  }
  
  lines_t* current = lines;
  while(current != NULL){
    printf("Line: ");
    
    switch(current->type){
      case EXPR:
        printf("EXPR:  ");
        print_expression((expr_node_t*)current->line);
        break;
      case BRANCH:
        printf("BRANCH:  ");
        {
          branch_t* branch = (branch_t*)current->line;
          if(branch != NULL){
            printf("name=");
            if(branch->name != NULL){
              print_single_token(branch->name);
            }
            printf(" start=");
            print_expression(branch->start_address);
            printf(" end=");
            print_expression(branch->end_address);
          }
        }
        break;
      case SCOPE:
        printf("SCOPE\n");
        print_scope((scope_t*)current->line);
        printf("End SCOPE\n");
        break;
      case EMPTY:
        printf("EMPTY\n");
        break;
      default:
        printf("UNKNOWN TYPE");
        break;
    }
    
    printf("\n");
    current = current->next;
  }
}