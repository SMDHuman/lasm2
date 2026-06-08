//-----------------------------------------------------------------------------
// lasm2_tokenizer.c
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#include "lasm2_tokenizer.h"
#include "hh_darray.h"

char newline_string[] = ";";

//-----------------------------------------------------------------------------
int load_input_file(char* input_name, lasm_file_t* file){
  FILE *input_file = fopen(input_name, "r");
	file->name = input_name;
  if(input_file == NULL){
    printf("[ERROR] Failed to open input file '%s'\n", input_name);
    return -1;
  }
	// Read the content of the file
  fseek(input_file, 0, SEEK_END);
  file->size = ftell(input_file);
  fseek(input_file, 0, SEEK_SET);
	file->text = (char*)malloc(file->size + 3); // Third is the charm
  fread(file->text, 1, file->size, input_file);
  file->text[file->size] = '\n';
  file->size++;
  file->text[file->size+1] = 0;
  fclose(input_file);
	//...
	for (size_t i = 0; i < file->size; i++){
    if(file->text[i] == '\n') file->line_count++;
  }
  file->line_count++;
	//...
  return 0;
}

//-----------------------------------------------------------------------------
uint8_t lasm_tokenizer(lasm_file_t* file, token_t** tokens){
	hh_darray_t *tokens_array = (hh_darray_t*)malloc(sizeof(hh_darray_t));
	hh_darray_init(tokens_array, sizeof(token_t));
	uint32_t line = 1, col = 0;
	uint8_t comment_counter = 0;
	//size_t i = 0;
	char *file_text = file->text - 1; 
	size_t iEOF = file->size + (size_t)file->text;
	while(1){
		//...
		token_t token = {0};
		token.origin = file;
		//...
		file_text++;
		col++;
		// Leave on end of file
		if(file_text >= (char*)iEOF) break;
		// Count lines 
		token.line = line;	
		if(file_text[0] == '\n'){
			line++;
			col = 1;
		}
		// Ignore comments
		if(file_text[0] == '\n' && comment_counter == 2) {comment_counter = 0; }
		if(file_text[0] != '\n' && comment_counter == 2) {col++; continue;}
		if(file_text[0] == '/' && comment_counter == 0) {comment_counter = 1; col++; continue;}
		if(file_text[0] == '/' && comment_counter == 1) {comment_counter = 2; col++;continue;}
		//=================================
		// Check chars
		if(file_text[0] == '<'){
						if(file_text[1] == '<'){
							token.id = BITSHIFT_L;
							token.text = file_text;
							token.text_size = 2;
						}
						else if(file_text[1] == '#'){
							token.id = MACRO_O;
							token.text = file_text;
							token.text_size = 2;
						}
						else if(file_text[1] == '='){
							token.id = EQ_SMALLER;
							token.text = file_text;
							token.text_size = 2;
						}
						else{
							token.id=SMALLER;
							token.text = file_text;
							token.text_size = 1;
						}
		}
		if(file_text[0] == '>'){
						if(file_text[1] == '>'){
							token.id = BITSHIFT_R;
							token.text = file_text;
							token.text_size = 2;
						}
						else if(file_text[1] == '='){
							token.id = EQ_GREATER;
							token.text = file_text;
							token.text_size = 2;
						}
						else{
							token.id=GREATER;
							token.text = file_text;
							token.text_size = 1;
						}
					}

		if(file_text[0] == '(') {token.id=RBRAC_O; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == ')') {token.id=RBRAC_C; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '[') {token.id=SBRAC_O; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == ']') {token.id=SBRAC_C; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '{') {token.id=CBRAC_O; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '}') {token.id=CBRAC_C; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '$') {token.id=DOLLAR; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '#') {
						if(file_text[1] == '>'){
							token.id = MACRO_C;
							token.text = file_text;
							token.text_size = 2;
						}
						else{
							token.id=HASH; 
							token.text = file_text; 
							token.text_size = 1;
						}
		}
		if(file_text[0] == ':'){
						if(file_text[1] == ':'){
							token.id = RANGE;
							token.text = file_text;
							token.text_size = 2;
						}else{
							token.id=COLON;
							token.text = file_text;
							token.text_size = 1;
						}
					}
		if(file_text[0] == '+') {token.id=PLUS; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '-') {token.id=MINUS; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '/') {token.id=SLASH; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '\\') {token.id=BSLASH; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '*') {token.id=ASTERISK; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '&') {token.id=BITW_AND; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '|') {token.id=BITW_OR; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '^') {token.id=BITW_XOR; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '?') {token.id=QUEST; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '!') {token.id=EXCLA; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '.') {token.id=DOT; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == ',') {token.id=COMMA; token.text = file_text; token.text_size = 1;}
		if(file_text[0] == '\n' || file_text[0] == ';'){
			token_t t; hh_darray_get(tokens_array, hh_darray_get_item_fill(tokens_array)-1, &t);
			if(t.id != NEWLINE){
				token.id=NEWLINE;
				file_text[0] = ';';
				token.text = file_text;
				token.text_size = 1; 
			}
		}
		//=================================
		// Check for numbers
		if(is_inside(file_text[0], "0123456789")){
			token.id = NUMBER;
			token.text = file_text;
			token.text_size = 1;
			while(is_inside(file_text[token.text_size], "0123456789abcdefxb") && file_text+token.text_size < (char*)iEOF){
				token.text_size++;
			}
		}
		// Check for strings
		if(file_text[0] == '"'){
			token.id = STRING_DB;
			token.text = file_text;
			token.text_size = 1;
			while(1){
				if(file_text[token.text_size] == '"' && file_text[token.text_size-1] != '\\') {
					token.text_size++;
					break;
				}
				if(file_text[token.text_size] == '\n' || file_text+token.text_size >= (char*)iEOF) {
					print_error_loc(&token);
					printf("Unterminated string\n");
					return -1;
				}
				token.text_size++;
			}
		}
		if(file_text[0] == '\''){
			token.id = STRING_SG;
			token.text = file_text;
			token.text_size = 1;
			while(1){
				if(file_text[token.text_size] == '\'' && file_text[token.text_size-1] != '\\') {
					token.text_size++;
					break;
				}
				if(file_text[token.text_size] == '\n' || file_text+token.text_size >= (char*)iEOF) {
					print_error_loc(&token);
					printf("Unterminated string\n");
					return -1;
				}
				token.text_size++;
			}
		}
		// Check for words
		if(is_alpha(file_text[0]) || file_text[0] == '_'){
			token.id = WORD;
			token.text = file_text;
			token.text_size = 1;
			while(is_alphanum(file_text[token.text_size]) || file_text[token.text_size] == '_'){
				token.text_size++;
			}
		}

		// Finish token and append it to the array
		if(token.text_size > 0){
			// print_single_token(&token);printf("\n");
			if(token.text_size > 1) {
				file_text+=token.text_size-1; 
				col+=token.text_size-1;
			}
			token.col=col-token.text_size;
			if(token.id == STRING_DB || token.id == STRING_SG){
				token.text_size-=2; // account for the quotes
				token.text +=	1; // account for the opening quote
			}
			hh_darray_append(tokens_array, &token);	
		}
	}
	// prepare output
	token_t *tokens_data = (token_t*)malloc((hh_darray_get_item_fill(tokens_array) + 1) * sizeof(token_t));
	for(size_t i = 0; i < hh_darray_get_item_fill(tokens_array); i++){
		hh_darray_get(tokens_array, i, &tokens_data[i]);
	}
	tokens_data[hh_darray_get_item_fill(tokens_array)] = (token_t){.id = EOT, .origin = file, .line = line, .col = col, .text = NULL, .text_size = 0};
	*tokens = tokens_data;
	hh_darray_deinit(tokens_array);
	free(tokens_array);
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t is_alpha(char c){
	if(((uint8_t)c <= 90 && (uint8_t)c >= 65) || 
	((uint8_t)c <= 122 && (uint8_t)c >= 97)) return 1;
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t is_alphanum(char c){
	if(((uint8_t)c <= 90 && (uint8_t)c >= 65) || 
	((uint8_t)c <= 122 && (uint8_t)c >= 97) || ((uint8_t)c <= 57 && (uint8_t)c >= 48)) return 1;
	return 0;
}
//-----------------------------------------------------------------------------
uint8_t is_inside(char c, const char* chars){
	for(uint16_t i = 0; chars[i] != 0; i++){
		if(c == chars[i]) return c;
	}
	return 0;
}
//-----------------------------------------------------------------------------
char char_upper(char c){
	if((uint8_t)c <= 122 && (uint8_t)c >= 97) return (char)((uint8_t)c - 32);
	return c;
}
//-----------------------------------------------------------------------------
char char_lower(char c){
	if((uint8_t)c <= 90 && (uint8_t)c >= 65) return (char)((uint8_t)c + 32);
	return c;
}
//-----------------------------------------------------------------------------
void print_warning_loc(token_t *token){
	// Print the offending token
	printf("[WARNING] '%s':%d:%d: ", token->origin->name, token->line, token->col);
}
void print_error_loc(token_t *token){
	// Print the offending token
	//printf("[DEBUG] Offending token: '%s'\n", token->text);
	//printf("[DEBUG] ID: %s\n", token_id_to_string(token->id));
	printf("[ERROR] '%s':%d:%d: ", token->origin->name, token->line, token->col);
}
void print_single_token(token_t *token){
	if(token->parent_copy){
		 print_single_token(token->parent_copy);
		 printf(" -> ");
	}
	char *text = malloc(token->text_size + 1);
	memcpy(text, token->text, token->text_size);
	text[token->text_size] = 0;
	printf("['%s', %s, %d:%d, %s]", token->origin->name ,token_id_to_string(token->id), token->line, token->col, text);
	free(text);
}
void print_tokens_as_code(hh_darray_t* tokens){
	token_t token;
	uint32_t head = 0;
	const uint32_t tokens_end = hh_darray_get_item_fill(tokens);
	while(head < tokens_end){
		hh_darray_get(tokens, head++, &token);
		print_single_token(&token);
		if(token.id == NEWLINE) printf("\n");
		else printf(" ");
	}		
	printf("\n");
}
const char* token_id_to_string(TOKEN_ID id){
	switch(id){
		case NONE: return "NONE";
		case EOT: return "EOT";
		case WORD: return "WORD";
		case NUMBER: return "NUMBER";
		case STRING_DB: return "STRING_DB";
		case STRING_SG: return "STRING_SG";
		case RBRAC_O: return "RBRAC_O";
		case RBRAC_C: return "RBRAC_C";
		case CBRAC_O: return "CBRAC_O";
		case CBRAC_C: return "CBRAC_C";
		case SBRAC_O: return "SBRAC_O";
		case SBRAC_C: return "SBRAC_C";
		case MACRO_O: return "MACRO_O";
		case MACRO_C: return "MACRO_C";
		case MACRO_ARG: return "MACRO_ARG";
		case NEWLINE: return "NEWLINE";
		case HASH: return "HASH";
		case COLON: return "COLON";
		case PLUS: return "PLUS";
		case MINUS: return "MINUS";
		case SLASH: return "SLASH";
		case BITSHIFT_L: return "BITSHIFT_L";
		case BITSHIFT_R: return "BITSHIFT_R";
		case BITW_OR: return "BITW_OR";
		case BITW_AND: return "BITW_AND";
		case BITW_XOR: return "BITW_XOR";
		case BSLASH: return "BSLASH";
		case ASTERISK: return "ASTERISK";
		case QUEST: return "QUEST";
		case EXCLA: return "EXCLA";
		case DOT: return "DOT";
		case COMMA: return "COMMA";
		case RANGE: return "RANGE";
		case DOLLAR: return "DOLLAR";
		case SMALLER: return "SMALLER";
		case GREATER: return "GREATER";
		case EQ_SMALLER: return "EQ_SMALLER";
		case EQ_GREATER: return "EQ_GREATER";
		default: return "UNKNOWN";
	}
}
