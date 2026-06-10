//-----------------------------------------------------------------------------
//lasm2_parser.c
//github.com/SMDHuman/lasm2
//-----------------------------------------------------------------------------
#include "lasm2_parser.h"


int parse_expression(token_t* tokens, expr_node_t* expr);
int parse_scope(token_t* tokens, scope_t* scope);
int parse_line(token_t* tokens, lines_t* lines);

//-----------------------------------------------------------------------------
void print_expression(expr_node_t* expr){
  if(expr == NULL){
    printf("NULL");
    return;
  }
  
  if(expr->left != NULL){
    printf("(");
    print_expression(expr->left);
    printf(")");
  }
  
  if(expr->token != NULL){
    print_single_token(expr->token);
  }
  
  if(expr->right != NULL){
    printf("(");
    print_expression(expr->right);
    printf(")");
  }
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
        printf("EXPR - ");
        print_expression((expr_node_t*)current->line);
        break;
      case BRANCH:
        printf("BRANCH - ");
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
      default:
        printf("UNKNOWN TYPE");
        break;
    }
    
    printf("\n");
    current = current->next;
  }
}