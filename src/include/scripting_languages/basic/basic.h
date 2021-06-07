#pragma once

struct keyword_token {
	const char* keyword;
	int token;
};

enum {
	NO_KEYWORD,
	KEYWORD_PRINT,
	KEYWORD_LIST,
	KEYWORD_CLS
};

struct keyword_token keywords[] {
	{"PRINT", KEYWORD_PRINT},
	{"LIST", KEYWORD_LIST},
	{"CLS", KEYWORD_CLS}
};


enum token_type_t {
	ERROR,
	FUNCTION,
	STRING,
	LABEL
};

struct token_t {
	token_type_t type;
	int keyword;
	char* contents; //If the token contains a string or an expression
	int line_num;
};


void BASIC_printf(const char* fmt, ...);
void BASIC_error(const char* error);
void BASIC_error(const char* error, int line_num);
void BASIC_not_implemented(const char* text, int line_num);
void BASIC_clear_screen();

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
		int basic_keyword_cls();
	public:
		BASIC();
		void eval_BASIC(const char* code);
};