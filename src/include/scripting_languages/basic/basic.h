#pragma once

enum token_type_t {
	error = 0,
	function = 1,
	string = 2,
	label = 3
};

void eval_BASIC(const char* code);