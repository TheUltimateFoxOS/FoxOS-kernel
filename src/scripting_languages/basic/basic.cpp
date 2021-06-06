#include <scripting_languages/basic/basic.h>

#include <string.h>

#include <driver/serial.h>

#include <renderer/font_renderer.h>

#include <paging/page_frame_allocator.h>

#include <memory/heap.h>

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


BASIC::BASIC() {
	tokens = (token_t*)malloc(sizeof(token_t));
	token_num = 0;
	token_iterator = 0;
	prog_code = "";
}

token_t BASIC::to_token(char* buffer, token_type_t type, int line_num) {
	token_t token;
	token.type = type;
	token.contents = (char*)malloc(sizeof(char) * strlen(buffer));
	memcpy(token.contents, buffer, strlen(buffer));
	token.line_num = line_num;
	return token;
}

int BASIC::lex_BASIC(const char* code) {
	prog_code = code;

	char* buffer = (char*)global_allocator.request_page();
	int buffer_len = 0;
	int line_num = 1;

	int state = 0;
	token_type_t last_token_type = token_type_t::function;

	if (tokens == NULL) {
		BASIC_error("Memory allocation failed!");
		global_allocator.free_page(buffer);

		return 1;
	}

	while(*code) {
		char str = *code++;

		token_t token;
		switch(str) {
			case '\n':
				if (state == 1) {
					BASIC_error("End of string not found!", line_num);
					global_allocator.free_page(buffer);

					return 1;
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

	token_num -= 1;
	return 0;
}

int BASIC::parse_BASIC() {
	int out_code = 0;

	while (token_iterator <= token_num) {
		token_t token = *(tokens+token_iterator);
		token_iterator++;

		if (token.type == token_type_t::error) {
			BASIC_error("Unknown error occured", token.line_num);
			out_code = -1;
			break;
		} else if (token.type == token_type_t::function) {
			int func_code;

			if (strcmp(token.contents, "PRINT") == 0 || strcmp(token.contents, "print") == 0) {
				func_code = basic_keyword_print(token);
			} else if (strcmp(token.contents, "LIST") == 0 || strcmp(token.contents, "list") == 0) {
				func_code = basic_keyword_list(token);
			} else {
				BASIC_error("Unknown token found", token.line_num);
				func_code = 1;
			}

			if (func_code != 0) {
				out_code = func_code;
				break;
			}
		} else if (token.type == token_type_t::string) {
			BASIC_error("Unexpected string found", token.line_num);
			out_code = 1;
			break;
		} else if (token.type == token_type_t::label) {
			BASIC_not_implemented("Labels are not implemented", token.line_num);
		}

		free(token.contents);
	}

	return out_code;
}

void BASIC::eval_BASIC(const char* code) {
	int out = lex_BASIC(code);

	if (out == 0) {
		int complete = parse_BASIC();
		BASIC_printf("BASIC program exited with code: %d\n", complete);
	}
	
	while (token_iterator <= token_num) {
		token_t token = *(tokens+token_iterator);
		token_iterator++;
		free(token.contents);
	}

	free(tokens);
}
