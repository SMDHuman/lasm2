//-----------------------------------------------------------------------------
//lasm2_parser.c
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#include "lasm2_parser.h"
#include "lasm2_tokenizer.h"
#include "lasm2_tokenreader.h"
#include "utils.h"

static float token_prc(token_t *token);
static expr_node_t* parse_expression_right(token_reader_t* reader, float min_prc);
static int parse_expression(token_reader_t* reader, expr_node_t* root_expr);
static int parse_scope(token_reader_t* reader, scope_t* scope);
static int parse_line(token_reader_t* reader, lines_t* lines, scope_t* parent);

//-----------------------------------------------------------------------------
static int parse_expression(token_reader_t* reader, expr_node_t* root_expr){
  expr_node_t* buffer_expr = parse_expression_right(reader, 1.);
  memcpy(root_expr, buffer_expr, sizeof(expr_node_t));
  free(buffer_expr);
  while(1){
    if(token_reader_EOF(reader)) break;
    if(token_reader_peek(reader, 0)->id == NEWLINE) break;
    if(token_reader_peek(reader, 0)->id == SBRAC_C) {token_reader_next(reader, 1); break;};
    if(token_reader_peek(reader, 0)->id == RBRAC_C) {token_reader_next(reader, 1); break;}
    if(token_prc(token_reader_peek(reader, 0)) <= 0) break;

    buffer_expr = parse_expression_right(reader, 1.);
    if(buffer_expr->left != NULL) free(buffer_expr->left);
    buffer_expr->left = NEW(expr_node_t, 1);
    memcpy(buffer_expr->left, root_expr, sizeof(expr_node_t));
    memcpy(root_expr, buffer_expr, sizeof(expr_node_t));
    free(buffer_expr);
    
  }
  // print_single_token(token_reader_peek(reader, 0));printf("\n");

  return 0;
}

static expr_node_t* parse_expression_right(token_reader_t* reader, float min_prc){
  // print_single_token(token_reader_peek(reader, 0));printf("\n");
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
  //...
  if(token_reader_peek(reader, 0)->id == RBRAC_O){
    token_reader_next(reader, 1);
    parse_expression(reader, new_expr);
  }
  //...
  if(token_reader_peek(reader, 0)->id == SBRAC_O){
    token_t* index_token = token_reader_next(reader, 1);
    expr_node_t* new_op_expr= NEW(expr_node_t, 1);
    new_op_expr->left = new_expr;
    new_op_expr->token = index_token;
    new_op_expr->right = NEW(expr_node_t, 1);
    parse_expression(reader, new_op_expr->right);
    new_expr = new_op_expr;
  }
  // Check for prefixes
  if(min_prc > 1 && token_prc(token_reader_peek(reader, 0)) > 0 && new_expr->token == NULL){
    new_expr->token = token_reader_next(reader, 1);
    new_expr->left = parse_expression_right(reader, 99999);
    new_expr->right = 0;
  }
  // EXIT ON ']' ')'
  if(token_reader_peek(reader, 0)->id == RBRAC_C || token_reader_peek(reader, 0)->id == SBRAC_C) {
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
static int parse_scope(token_reader_t* reader, scope_t* scope){
  if(token_reader_peek(reader, 0)->id == DOT){
    token_reader_next(reader, 1);
    if(!token_reader_expect(reader, WORD, 0)) return -1;
    branch_t* branch = NEW(branch_t, 1);
    branch->start_address = NULL; branch->end_address = NULL;
    scope->header = branch;
    branch->name = token_reader_next(reader, 1);
    //...
    if(token_reader_peek(reader, 0)->id == SBRAC_O){
      branch->start_address = NEW(expr_node_t, 1);
      token_reader_next(reader, 1);
      int res = parse_expression(reader, branch->start_address);
      if(res) return res;
      if(token_reader_peek(reader, 0)->id == RANGE){
        token_reader_next(reader, 1);
        branch->end_address = NEW(expr_node_t, 1);
        int res = parse_expression(reader, branch->end_address);
        if(res) return res;
      }
    }
  }else{
    scope->header = NULL;
  }
  //...
  token_reader_skip_until_not(reader, NEWLINE);
  if(token_reader_peek(reader, 0)->id == CBRAC_O){
    token_reader_next(reader, 1);
    scope->lines = NEW(lines_t, 1);
    int res = parse_line(reader, scope->lines, scope);
    if(res) return res;
  }
  return 0;
}

int lasm2_parser(token_t* tokens, lines_t* lines){
  token_reader_t* reader = new_token_reader(tokens);
  int res = parse_line(reader, lines, NULL);
  free_token_reader(reader);
  return res;
}

static int parse_line(token_reader_t* reader, lines_t* lines, scope_t* parent){
  if(lines == NULL) return -1;
  lines->line = NULL;
  lines->next = NULL;
  lines->type = EMPTY;
  lines_t* root_line = lines;
  lines_t* current_line = root_line;
  while(!token_reader_EOF(reader)){
  // print_single_token(token_reader_peek(reader, 0));printf("\n");

    // Parse as scope
    if(token_reader_peek(reader, 0)->id == NEWLINE){
      token_reader_next(reader, 1);
    }
    else if(token_reader_peek(reader, 0)->id == CBRAC_C){
      token_reader_next(reader, 1);
      return 0;
    }
    else if(token_reader_peek(reader, 0)->id == DOT || token_reader_peek(reader, 0)->id == CBRAC_O){
      scope_t* scope = NEW(scope_t, 1);
      scope->parent = parent;
      scope->lines = NULL;
      scope->header = NULL;
      int res = parse_scope(reader, scope);
      if(res) return res;
      current_line->type = SCOPE;
      current_line->line = scope;
      current_line->next = NEW(lines_t, 1);
      current_line = current_line->next;
      current_line->type = EMPTY;
      current_line->next = 0;
      current_line->line = 0;
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
  return 0;
}

//-----------------------------------------------------------------------------
static float token_prc(token_t *token){
  float precedence = 1;
  if(token->id == COLON) return precedence;
  if(token->id == QUEST) return precedence;
  precedence++;
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
void print_expression(expr_node_t* expr, int indent){
  if(expr == NULL){
    printf("NULL");
    return;
  }
  if(expr->left || expr->right) printf("(");
  if(expr->token != NULL){
    char text[expr->token->text_size + 1];
    memcpy(text, expr->token->text, expr->token->text_size);
    text[expr->token->text_size] = 0;
    printf(" %s ", text);
  }
  if(expr->left != NULL){
    print_expression(expr->left, indent);
  }
  
  if(expr->right != NULL){
    print_expression(expr->right, indent);
  }
  if(expr->left || expr->right) printf(")");
}
//-----------------------------------------------------------------------------
void print_scope(scope_t* scope, int indent){
  if(scope == NULL){
    printf("NULL scope\n");
    return;
  }
  
  for(int i = 0; i < indent; i++) printf("  ");
  printf("Scope: ");
  if(scope->header != NULL){
    printf("name=");
    if(scope->header->name != NULL){
        token_t* token = scope->header->name;
        char text[token->text_size + 1];
        memcpy(text, token->text, token->text_size);
        text[token->text_size] = 0;
        printf(" %s ", text);
    }
    printf(", start=");
    print_expression(scope->header->start_address, indent);
    printf(", end=");
    print_expression(scope->header->end_address, indent);
  }
  if(scope->parent != NULL){
    printf(", Parent name=");
    if(scope->parent->header != NULL){
      if(scope->parent->header->name != NULL){
        token_t* token = scope->parent->header->name;
        char text[token->text_size + 1];
        memcpy(text, token->text, token->text_size);
        text[token->text_size] = 0;
        printf(" %s ", text);
      }
    }
  }
  printf("\n");
  
  if(scope->lines != NULL){
    print_line(scope->lines, indent + 1);
  }
}
//-----------------------------------------------------------------------------
void print_line(lines_t* lines, int indent){
  if(lines == NULL){
    printf("NULL line\n");
    return;
  }
  
  for(int i = 0; i < indent; i++) printf("  ");
  printf("Line: ");
  
  if(lines->type == EMPTY){
    printf("EMPTY\n");
    return;
  } else if(lines->type == EXPR){
    printf("EXPR:  ");
    print_expression((expr_node_t*)lines->line, indent);
    printf("\n");
  } else if(lines->type == SCOPE){
    printf("SCOPE\n");
    print_scope((scope_t*)lines->line, indent + 1);
    for(int i = 0; i < indent; i++) printf("  ");
    printf("End SCOPE\n");
  } else {
    printf("UNKNOWN TYPE\n");
  }
  if(lines->next){
    print_line(lines->next, indent);
  }
}

//-----------------------------------------------------------------------------
void free_expr_node(expr_node_t* expr){
  if(expr == NULL) return;
  
  // Recursively free left and right subtrees
  if(expr->left != NULL){
    free_expr_node(expr->left);
  }
  if(expr->right != NULL){
    free_expr_node(expr->right);
  }
  
  // Free the node itself
  free(expr);
}

//-----------------------------------------------------------------------------
void free_branch(branch_t* branch){
  if(branch == NULL) return;
  
  // Free start and end address expressions
  if(branch->start_address != NULL){
    free_expr_node(branch->start_address);
  }
  if(branch->end_address != NULL){
    free_expr_node(branch->end_address);
  }
  
  // Free the branch itself
  free(branch);
}

//-----------------------------------------------------------------------------
void free_scope(scope_t* scope){
  if(scope == NULL) return;
  
  // Free header (branch)
  if(scope->header != NULL){
    free_branch(scope->header);
  }
  
  // Free lines within scope
  if(scope->lines != NULL){
    free_lines(scope->lines);
  }
  
  // Don't free parent - it's managed elsewhere
  
  // Free the scope itself
  free(scope);
}

//-----------------------------------------------------------------------------
void free_lines(lines_t* lines){
  if(lines == NULL) return;
  
  // Save next pointer before freeing current
  lines_t* next = lines->next;
  
  // Free the current line based on its type
  if(lines->type == EXPR){
    free_expr_node((expr_node_t*)lines->line);
  }
  else if(lines->type == SCOPE){
    free_scope((scope_t*)lines->line);
  }
  // EMPTY type has no allocation to free
  
  // Free the lines node itself
  free(lines);
  
  // Recursively free the rest of the linked list
  if(next != NULL){
    free_lines(next);
  }
}