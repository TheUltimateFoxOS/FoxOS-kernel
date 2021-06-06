#pragma once

enum token_type_t {
	error = 0,
	function = 1,
	string = 2,
	label = 3
};

struct token_t {
	token_type_t type;
	char* contents;
	int line_num;
};

struct lexer_out {
	token_t* tokens;
	int token_num;
	int failed;
};

void BASIC_printf(const char* fmt, ...);
void BASIC_error(const char* error);
void BASIC_error(const char* error, int line_num);
void BASIC_not_implemented(const char* text, int line_num);

class BASIC {
	private:
		token_t* tokens;
		int token_num;
		int token_iterator;
		const char* prog_code;

		token_t to_token(char* buffer, token_type_t type, int line_num);

		int lex_BASIC(const char* code);
		int parse_BASIC();

		//IO
		int basic_keyword_print(token_t token);
		int basic_keyword_list(token_t token);
	public:
		BASIC();
		void eval_BASIC(const char* code);
};