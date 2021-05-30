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
};


void eval_BASIC(const char* code);