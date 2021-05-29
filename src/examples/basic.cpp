#include <scripting_languages/basic/basic.h>

void basic_test() {
	eval_BASIC("PRINT \"Hello World\"\ntest:\nPRINT \"This is a test!\"");
}