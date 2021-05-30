#include <scripting_languages/basic/basic.h>
#include <driver/serial.h>
#include <paging/page_frame_allocator.h>
#include <memory/heap.h>

void BASIC_error(const char* error) {
	driver::global_serial_driver->printf("Error: %s", error);
}

void BASIC_error(const char* error, int line_num) {
	driver::global_serial_driver->printf("Error line %d: %s", line_num, error);
}

token_t to_token(char* buffer, token_type_t type, int line_num) {
	token_t token;
	token.type = type;
	token.contents = buffer;
	token.line_num = line_num;
	return token;
}

lexer_out* lex_BASIC(const char* code) {
	char* buffer = (char*)global_allocator.request_page();
	int buffer_len = 0;
	int line_num = 1;

	int state = 0;
	token_type_t last_token_type = token_type_t::function;

	token_t* tokens = (token_t*)malloc(sizeof(token_t));
	int token_num = 0;

	if (tokens == NULL) {
		BASIC_error("Memory allocation failed!");
		return NULL;
	}

	token_t token;
	while(*code) {
		char str = *code++;

		switch(str) {
			case '\n':
				if (state == 1) {
					BASIC_error("End of string not found!", line_num);
					global_allocator.free_page(buffer);
					free(tokens);
					return NULL;
				}

				token = to_token(buffer, last_token_type, line_num);
				if (token_num != 0) {
					tokens = (token_t*)realloc(tokens, token_num * sizeof(token_t), (token_num + 1) * sizeof(token_t));
				}
				*(tokens+token_num) = token;
				token_num++;

				memset(buffer, 0, 4096);
				last_token_type = token_type_t::function;
				buffer_len = 0;
				line_num++;
				break;
			case ' ':
				if (state == 0) {
					token = to_token(buffer, last_token_type, line_num);
					if (token_num != 0) {
						tokens = (token_t*)realloc(tokens, token_num * sizeof(token_t), (token_num + 1) * sizeof(token_t));
					}
					*(tokens+token_num) = token;
					token_num++;

					memset(buffer, 0, 4096);
					last_token_type = token_type_t::function;
					buffer_len = 0;
				} else {
					buffer[buffer_len] = str;
					buffer_len++;
				}
				break;
			case '\"':
				if (state == 1) {
					state = 0;
					last_token_type = token_type_t::string;
				} else {
					state = 1;
				}
				break;
			case ':':
				if (state == 1) {
					buffer[buffer_len] = str;
					buffer_len++;
					break;
				}

				last_token_type = token_type_t::label;
				break;
			default:
				buffer[buffer_len] = str;
				buffer_len++;
				break;
		}
	}
	if (buffer_len != 0) {
		token = to_token(buffer, last_token_type, line_num);
		if (token_num != 0) {
			tokens = (token_t*)realloc(tokens, token_num * sizeof(token_t), (token_num + 1) * sizeof(token_t));
		}
		*(tokens+token_num) = token;
		token_num++;
	}

	global_allocator.free_page(buffer);

	lexer_out* out;
	out->tokens = tokens;
	out->token_num = token_num - 1;
	return out;
}

void parse_BASIC(token_t* tokens, int token_num) {
	int i = 0;

	while (i <= token_num) {
		token_t token = *(tokens+i);
		i++;

		if (token.type == token_type_t::error) {
			BASIC_error("Unknown error occured", token.line_num);
		} else if (token.type == token_type_t::function) {
			driver::global_serial_driver->printf("FUNCTION\n");
		} else if (token.type == token_type_t::string) {
			driver::global_serial_driver->printf("STRING\n");
		} else if (token.type == token_type_t::label) {
			driver::global_serial_driver->printf("LABEL\n");
		}
	}
}

void eval_BASIC(const char* code) {
	lexer_out* out = lex_BASIC(code);

	if (out->tokens == NULL) {
		free(out->tokens);
		return;
	} else if (out->token_num == 0) {
		free(out->tokens);
		return;
	}

	parse_BASIC(out->tokens, out->token_num);
	
	free(out->tokens);
}
