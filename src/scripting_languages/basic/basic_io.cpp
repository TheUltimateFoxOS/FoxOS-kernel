#include <scripting_languages/basic/basic.h>

#include <memory/heap.h>

int BASIC::basic_keyword_print(token_t token) {
	token_t string_token = *(tokens+token_iterator);
	token_iterator++;

	if (string_token.line_num == token.line_num) {
		if (string_token.type == token_type_t::STRING) {
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
	return 0;
}

int BASIC::basic_keyword_list(token_t token) {
	if (prog_code == "") {
		BASIC_error("Unable to get program contents", token.line_num);
		return 1;
	}

	BASIC_printf("%s", prog_code);
	return 0;
}

int BASIC::basic_keyword_cls() {
	BASIC_clear_screen();
	return 0;
}