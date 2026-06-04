#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"
#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_macro.h"

typedef struct {
  lasm_file_t input_file;
  FILE *output_file;
  char *include_path;
  token_t *tokens;
  macro_t *macros;
} assembler_t;

static int load_input_file(assembler_t *assembler);
void help_command(const char *invoc_name);

//----------------------------------------------------------------------------- 
int main(int argc, char *argv[]){
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
  assembler.output_file = fopen(hh_argparse_get_op_short_or_long(parser, 'o', "output"), "w");
  assembler.include_path = hh_argparse_get_op_short_or_long(parser, 'i', "include");

  // Unpack input file
  if(load_input_file(&assembler) != 0){
    exit_code = -1;
    goto exit_assembler;
  }

  if(lasm_tokenizer(&assembler.input_file, &assembler.tokens) != 0){
    exit_code = -1;
    goto exit_assembler;
  }

  if( lasm_parse_macro(assembler.tokens, &assembler.macros) != 0){
    exit_code = -1;
    goto exit_assembler;
  }

  for (size_t i = 0; assembler.tokens[i].id != EOT; i++){
    print_single_token(&assembler.tokens[i]);
    printf("\n");
  }

  exit_assembler:
  // Deinitialize everything
  hh_argparse_deinit(parser);
  if(assembler.output_file) fclose(assembler.output_file);
  if(assembler.input_file.text) free(assembler.input_file.text);

  return exit_code;
}
//-----------------------------------------------------------------------------
static int load_input_file(assembler_t *assembler){
  FILE *input_file = fopen(assembler->input_file.name, "r");
  if(input_file == NULL){
    printf("[ERROR] Failed to open input file '%s'\n", assembler->input_file.name);
    return -1;
  }

  fseek(input_file, 0, SEEK_END);
  assembler->input_file.size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);
  assembler->input_file.text = (char*)malloc(assembler->input_file.size + 1);
  fread(assembler->input_file.text, 1, assembler->input_file.size, input_file);
  assembler->input_file.text[assembler->input_file.size] = 0;
  fclose(input_file);

  for (size_t i = 0; i < assembler->input_file.size; i++){
    if(assembler->input_file.text[i] == '\n') assembler->input_file.line_count++;
  }
  assembler->input_file.line_count++;

  return 0;
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