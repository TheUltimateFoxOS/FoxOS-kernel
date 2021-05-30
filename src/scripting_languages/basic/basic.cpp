#include <scripting_languages/basic/basic.h>
#include <driver/serial.h>
#include <renderer/font_renderer.h>
#include <paging/page_frame_allocator.h>
#include <memory/heap.h>
#include <string.h>

void BASIC_printf(const char* fmt, ...) {
	va_list ap;
	const char* s;
	unsigned long n;

	va_start(ap, fmt);
	renderer::global_font_renderer->vsprintf(fmt, ap);
	va_end(ap);
}

void BASIC_error(const char* error) {
	BASIC_printf("Error: %s\n", error);
}

void BASIC_error(const char* error, int line_num) {
	BASIC_printf("Error line %d: %s\n", line_num, error);
}

void BASIC_not_implemented(const char* text, int line_num) {
	BASIC_printf("Not implemented line %d: %s\n", line_num, text);
}

token_t to_token(char* buffer, token_type_t type, int line_num) {
	token_t token;
	token.type = type;
	token.contents = (char*)malloc(sizeof(char) * strlen(buffer));
	memcpy(token.contents, buffer, strlen(buffer));
	token.line_num = line_num;
	return token;
}

lexer_out lex_BASIC(const char* code) {
	char* buffer = (char*)global_allocator.request_page();
	int buffer_len = 0;
	int line_num = 1;

	int state = 0;
	token_type_t last_token_type = token_type_t::function;

	token_t* tokens = (token_t*)malloc(sizeof(token_t));
	int token_num = 0;

	lexer_out out;
	out.failed = 1;

	if (tokens == NULL) {
		BASIC_error("Memory allocation failed!");
		return out;
	}

	while(*code) {
		char str = *code++;

		token_t token;
		switch(str) {
			case '\n':
				if (state == 1) {
					BASIC_error("End of string not found!", line_num);
					global_allocator.free_page(buffer);
					free(tokens);
					return out;
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
		token_t token = to_token(buffer, last_token_type, line_num);
		if (token_num != 0) {
			tokens = (token_t*)realloc(tokens, token_num * sizeof(token_t), (token_num + 1) * sizeof(token_t));
		}
		*(tokens+token_num) = token;
		token_num++;
	}

	global_allocator.free_page(buffer);

	out.tokens = tokens;
	out.token_num = token_num - 1;
	out.failed = 0;
	return out;
}

int parse_BASIC(token_t* tokens, int token_num) {
	int i = 0;

	while (i <= token_num) {
		token_t token = *(tokens+i);
		i++;

		if (token.type == token_type_t::error) {
			BASIC_error("Unknown error occured", token.line_num);
			return -1;
		} else if (token.type == token_type_t::function) {
			if (strcmp(token.contents, "PRINT") == 0 || strcmp(token.contents, "print") == 0) {
				token_t string_token = *(tokens+i);
				i++;

				if (string_token.line_num == token.line_num) {
					if (string_token.type == token_type_t::string) {
						BASIC_printf("%s\n", string_token.contents);
					} else {
						BASIC_error("Tried to print a value that is not a string", string_token.line_num);
						return 1;
					}
				} else {
					BASIC_error("Could not find a value to print", token.line_num);
					return 1;
				}

				free(string_token.contents);
			} else {
				BASIC_error("Unknown token found", token.line_num);
				return 1;
			}
		} else if (token.type == token_type_t::string) {
			BASIC_error("Unexpected string found", token.line_num);
			return 1;
		} else if (token.type == token_type_t::label) {
			BASIC_not_implemented("Labels are not implemented", token.line_num);
		}

		free(token.contents);
	}

	return 0;
}

void eval_BASIC(const char* code) {
	lexer_out out = lex_BASIC(code);

	if (out.failed == 1) {
		return;
	} else if (out.tokens == NULL) {
		return;
	} else if (out.token_num == 0) {
		return;
	}

	int complete = parse_BASIC(out.tokens, out.token_num);
	BASIC_printf("BASIC program exited with code: %d\n", complete);
	
	free(out.tokens);
}
