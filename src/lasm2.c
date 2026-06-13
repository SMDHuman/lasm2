#include <time.h>
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"
#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_macro.h"
#include "lasm2_parser.h"
#include "utils.h"

typedef struct {
  lasm_file_t input_file;
  lasm_file_t* include_files;
  FILE *output_file;
  char** include_paths;
  token_t *tokens;
  macro_t *macros;
  // labels
  // scopes
} assembler_t;

void help_command(const char *invoc_name);

//----------------------------------------------------------------------------- 
int main(int argc, char *argv[]){
  struct timespec start_time; timespec_get(&start_time, TIME_UTC); 
  int exit_code = 0;
  // Parse Arguments
  hh_argparse_t *parser = hh_argparse_init(argc, argv);
  if(parser == NULL){
    printf("[ERROR] Argument parsing failed\n");
    return -1;
  }
  if(hh_argparse_check_op_short(parser, 'h') || hh_argparse_check_op_long(parser, "help") || argc == 1){
    help_command(argv[0]);
    hh_argparse_deinit(parser);
    return 0;
  }
  // Initialize assembler struct
  assembler_t assembler = {0};
  assembler.input_file.name = hh_argparse_get_positional(parser, 0);

  // Extract include paths
  assembler.include_paths = (char**)malloc(sizeof(char*)*2);
  char* last_slash = strchr(assembler.input_file.name, '/');
  //...
  int dir_name_size;
  const char* dir_path;
  if(last_slash == NULL){
    // No path separator found, use current directory
    dir_path = ".";
    dir_name_size = 1;
  } else {
    dir_path = assembler.input_file.name;
    dir_name_size = (int)(last_slash - assembler.input_file.name);
  }
  //...
  assembler.include_paths[0] = (char*)malloc(dir_name_size+1);
  memcpy(assembler.include_paths[0], dir_path, dir_name_size);
  assembler.include_paths[0][dir_name_size] = '\0';
  assembler.include_paths[1] = NULL;
  printf("Input file directory: %s\n", assembler.include_paths[0]);

  //assembler.include_paths = hh_argparse_get_op_short_or_long(parser, 'i', "include");

  char* output_filename = "a.out";
  if(hh_argparse_check_op_short(parser, 'o') || hh_argparse_check_op_long(parser, "output")){
    output_filename = hh_argparse_get_op_short_or_long(parser, 'o', "output");
  }
  assembler.output_file = fopen(output_filename, "w");
  // Unpack input file
  printf("[INFO] Reading input file\n");
  exit_code = load_input_file(assembler.input_file.name, &assembler.input_file);
  if(exit_code != 0){
    goto exit_assembler;
  }

  // Tokenize input file
  printf("[INFO] Tokenizing '%s'\n", assembler.input_file.name);
  exit_code = lasm_tokenizer(&assembler.input_file, &assembler.tokens);
  if(exit_code != 0){
    goto exit_assembler;
  }
  
  printf("[INFO] Applying macros\n");
  // Make multiple passes to slice macros
  for (int i = 0; i < 16; i++){
    // Parse macros
    exit_code = lasm_parse_macro(assembler.tokens, &assembler.macros);
    if(exit_code  != 0){
      goto exit_assembler;
    }
    // Generate tokens with macros
    exit_code = lasm_regenerate_tokens_with_macros(assembler.tokens, assembler.macros, &assembler.tokens, &assembler.include_files, assembler.include_paths);
    if(exit_code != 0){
      goto exit_assembler;
    }
  }

  lines_t* lines = NEW(lines_t, 1);
  printf("[INFO] Parsing\n");
  exit_code = lasm2_parser(assembler.tokens, lines);
  if(exit_code != 0){
    goto exit_assembler;
  }
  printf("[INFO] Lines parsed\n");
  print_line(lines, 1);
  
  // for (size_t i = 0; assembler.tokens[i].id != EOT; i++){
  //   print_single_token(&assembler.tokens[i]);
  //   printf("\n");
  // }
  

  exit_assembler:
  // Deinitialize everything
  hh_argparse_deinit(parser);
  //if(assembler.include_paths) free(assembler.include_paths);
  if(assembler.macros) free_macros(assembler.macros);
  if(assembler.output_file) fclose(assembler.output_file);
  if(assembler.input_file.text) free(assembler.input_file.text);
  if(assembler.tokens) free(assembler.tokens);
  if(assembler.macros) free(assembler.macros);
  //..,
  struct timespec end_time; timespec_get(&end_time, TIME_UTC); 
  if(exit_code != 0){
    printf("[ERROR] Assembling failed with code %d\n", exit_code);
  }else{
    printf("[INFO] Assembling completed successfully");
    printf(" (%ld ms)\n", (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000);
  }
  free_lines(lines);
  return exit_code;
}
//-----------------------------------------------------------------------------
void help_command(const char *invoc_name){
  printf("Usage: %s [options] <input_file>\n", invoc_name);
  printf("Options:\n");
  printf("  -h, --help       Show this help message\n");
  printf("  -o, --output     Specify output file\n");
  printf("  -i, --include     Specify include path\n");
  printf("  -L, --logisim     Change output format to raw 2.0 for logisim\n");
  printf("  -V, --verilog     Change output format to .mem for verilog\n");
}