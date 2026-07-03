#include <time.h>
#define HH_ARGPARSE_IMPLEMENTATION
#include "hh_argparse.h"
#define HH_DARRAY_IMPLEMENTATION
#include "hh_darray.h"
#include "lasm2_tokenizer.h"
#include "lasm2_macro.h"
#include "lasm2_parser.h"
#include "lasm2_assembler.h"
#include "utils.h"

typedef struct {
  lasm_file_t input_file;
  lasm_file_t* include_files;
  FILE *output_file;
  char** include_paths;
  token_t *tokens;
  macro_t *macros;
  lines_t *lines;
} assembler_t;

void help_command(const char *invoc_name);
int convert_to_logisim_format(const char *output_filename);
int convert_to_verilog_mem_format(const char *output_filename);

//----------------------------------------------------------------------------- 
int main(int argc, char *argv[]){
  struct timespec start_time; timespec_get(&start_time, TIME_UTC); 
  int exit_code = 0;
  assembly_t* assembly = NULL;
  // Parse Arguments
  hh_argparse_t *parser = hh_argparse_init(argc, argv);
  if(parser == NULL){
    printf("[ERROR] Argument parsing failed\n");
    return -1;
  }
  if(hh_argparse_check_op_short_or_long(parser, 'h', "help") || argc == 1){
    help_command(argv[0]);
    hh_argparse_deinit(parser);
    return 0;
  }
  // Initialize assembler struct
  assembler_t assembler = {0};
  assembler.input_file.name = hh_argparse_get_positional(parser, 0);

  // Extract include paths
  int num_include_paths = hh_argparse_get_op_count_short_or_long(parser, 'i', "include");
  assembler.include_paths = (char**)malloc(sizeof(char*)*(2 + num_include_paths));
  // Find the last slash in the input file name to determine the directory path
  char* last_slash = strchr(assembler.input_file.name, '/');
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
  // Include input file path
  assembler.include_paths[0] = (char*)malloc(dir_name_size+1);
  memcpy(assembler.include_paths[0], dir_path, dir_name_size);
  assembler.include_paths[0][dir_name_size] = '\0';
  // Include additional include paths
  for(int i = 0; i < num_include_paths; i++){
    assembler.include_paths[i+1] = strdup(hh_argparse_get_nth_op_short_or_long(parser, 'i', "include", i));
  }
  // Null terminate the include paths array
  assembler.include_paths[num_include_paths + 1] = NULL;
  printf("Input file directories:\n");
  for(int i = 0; assembler.include_paths[i]; i++){
    printf("    %s,\n", assembler.include_paths[i]);
  }

  char* output_filename = hh_argparse_get_op_short_or_long(parser, 'o', "output");;
  if(!output_filename) output_filename = "a.out";

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

  assembler.lines = NEW(lines_t, 1);
  printf("[INFO] Parsing\n");
  exit_code = lasm2_parser(assembler.tokens, assembler.lines);
  if(exit_code != 0){
    goto exit_assembler;
  }
  printf("[INFO] Lines parsed\n");
  print_line(assembler.lines, 1);
  
  // for (size_t i = 0; assembler.tokens[i].id != EOT; i++){
  //   print_single_token(&assembler.tokens[i]);
  //   printf("\n");
  // }
  
  //...
  printf("[INFO] Assembling\n");
  assembler.output_file = fopen(output_filename, "w");
  assembly = lasm2_assembly_new(assembler.lines, &(assembly_config_t){.branch_default_size=2,.out_file=assembler.output_file});
  exit_code = lasm2_assemble(assembly);
  if(exit_code != 0){
    goto exit_assembler;
  }
  //...
  printf("[INFO] Assembling Patches\n");
  exit_code = lasm2_assemble_patches(assembly);
  if(exit_code != 0){
    goto exit_assembler;
  }
  fclose(assembler.output_file); assembler.output_file = NULL;
  printf("[INFO] Lines assembled\n");

  // Convert to Logisim format if requested
  if(hh_argparse_check_op_short_or_long(parser, 'L', "logisim")){
    printf("[INFO] Converting to Logisim format\n");
    exit_code = convert_to_logisim_format(output_filename);
    if(exit_code != 0){
      goto exit_assembler;
    }
  }
  // Convert to Verilog .mem format if requested
  else if(hh_argparse_check_op_short_or_long(parser, 'V', "verilog")){
    printf("[INFO] Converting to Verilog .mem format\n");
    exit_code = convert_to_verilog_mem_format(output_filename);
    if(exit_code != 0){
      goto exit_assembler;
    }
  }

  //...
  exit_assembler:
  // Deinitialize everything
  hh_argparse_deinit(parser);
  printf("[INFO] Cleaning up\n");
  for(int i = 0; assembler.include_paths[i]; i++) free(assembler.include_paths[i]);
  if(assembler.include_paths) free(assembler.include_paths);
  if(assembler.macros) free_macros(assembler.macros);
  if(assembler.output_file) fclose(assembler.output_file);
  if(assembler.input_file.text) free(assembler.input_file.text);
  if(assembler.tokens) free(assembler.tokens);
  if(assembly) lasm2_assembly_free(assembly);
  if(assembler.lines) free_lines(assembler.lines);
  //...
  struct timespec end_time; timespec_get(&end_time, TIME_UTC); 
  if(exit_code != 0){
    printf("[ERROR] Assembling failed with code %d\n", exit_code);
  }else{
    printf("[INFO] Assembling completed successfully");
    printf(" (%ld ms)\n", (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_nsec - start_time.tv_nsec) / 1000000);
  }
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
//-----------------------------------------------------------------------------
int convert_to_logisim_format(const char *output_filename){
  // Open the output file for reading
  FILE *input_file = fopen(output_filename, "rb");
  if (!input_file) {
    printf("[ERROR] Failed to open output file '%s' for reading\n", output_filename);
    return -1;
  }

  // Create a temporary file for writing the Logisim format
  char temp_filename[256];
  snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", output_filename);
  FILE *temp_file = fopen(temp_filename, "wb");
  if (!temp_file) {
    printf("[ERROR] Failed to create temporary file '%s'\n", temp_filename);
    fclose(input_file);
    return -1;
  }

  // Write the Logisim header
  fprintf(temp_file, "v2.0 raw\n");

  // Read the original output file and write its contents in Logisim format
  int byte;
  int newline_count = 0;
  while ((byte = fgetc(input_file)) != EOF) {
    fprintf(temp_file, "%02x ", (unsigned char)byte);
    newline_count++;
    if (newline_count == 16) {
      fprintf(temp_file, "\n");
      newline_count = 0;
    }
  }

  // Close both files
  fclose(input_file);
  fclose(temp_file);

  // Replace the original output file with the temporary file
  if (remove(output_filename) != 0) {
    printf("[ERROR] Failed to remove original output file '%s'\n", output_filename);
    return -1;
  }
  if (rename(temp_filename, output_filename) != 0) {
    printf("[ERROR] Failed to rename temporary file '%s' to '%s'\n", temp_filename, output_filename);
    return -1;
  }

  return 0;
}

//-----------------------------------------------------------------------------
int convert_to_verilog_mem_format(const char *output_filename){
  // Open the output file for reading
  FILE *input_file = fopen(output_filename, "rb");
  if (!input_file) {
    printf("[ERROR] Failed to open output file '%s' for reading\n", output_filename);
    return -1;
  }

  // Create a temporary file for writing the Verilog .mem format
  char temp_filename[256];
  snprintf(temp_filename, sizeof(temp_filename), "%s.tmp", output_filename);
  FILE *temp_file = fopen(temp_filename, "wb");
  if (!temp_file) {
    printf("[ERROR] Failed to create temporary file '%s'\n", temp_filename);
    fclose(input_file);
    return -1;
  }

  // Read the original output file and write its contents in Verilog .mem format
  int byte;
  while ((byte = fgetc(input_file)) != EOF) {
    fprintf(temp_file, "%02x\n", (unsigned char)byte);
  }

  // Close both files
  fclose(input_file);
  fclose(temp_file);

  // Replace the original output file with the temporary file
  if (remove(output_filename) != 0) {
    printf("[ERROR] Failed to remove original output file '%s'\n", output_filename);
    return -1;
  }
  if (rename(temp_filename, output_filename) != 0) {
    printf("[ERROR] Failed to rename temporary file '%s' to '%s'\n", temp_filename, output_filename);
    return -1;
  }

  return 0;
}