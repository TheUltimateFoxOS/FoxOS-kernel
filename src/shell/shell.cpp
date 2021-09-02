#include <string.h>

#include <shell/shell.h>

#include <renderer/font_renderer.h>

using namespace shell;

Shell* shell::global_shell;

Shell::Shell() {
	
}

void add_char(char c);
bool is_reading();

void Shell::keypress(char key) {
	if (key == 0) {
		return;
	}

	if (is_reading()) {
		renderer::global_font_renderer->printf("%c", key);
		add_char(key);
		return;
	}
}
