#include "hh_argparse.h"
#include "lasm_tokenizer.h"

typedef struct {
  char *input_file_name;
  FILE *input_file;
  char *output_file_name;
  FILE *output_file;
  char *include_path;
  token_t *tokens;
  macro_t *macros;
} assembler_t;

void help_command(const char *program_invocation_name);

//-----------------------------------------------------------------------------
int main(int argc, char *argv[]){
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

  return 0;
}

//-----------------------------------------------------------------------------
void help_command(const char *program_invocation_name){
  printf("Usage: %s [options] <input_file>\n", program_invocation_name);
  printf("Options:\n");
  printf("  -h, --help       Show this help message\n");
  printf("  -o, --output     Specify output file\n");
  printf("  -i, --include     Specify include path\n");
  printf("  -L, --logisim     Change output format to raw 2.0 for logisim\n");
  printf("  -V, --verilog     Change output format to .mem for verilog\n");
}