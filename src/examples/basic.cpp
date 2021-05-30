#include <scripting_languages/basic/basic.h>

void basic_test() {
	extern const char basic_prog[];

	eval_BASIC((char*) basic_prog);
}