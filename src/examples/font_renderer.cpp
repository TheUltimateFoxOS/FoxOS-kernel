#include <renderer/font_renderer.h>

void font_renderer_test() {
	renderer::global_font_renderer->printf("Im now colored %fString: %s, Hex: 0x%x, Dec: %d, Char: %c %rand now im white\n\n", 0xff00ff00, "Hello World!", 0xf00d, 1001, 'X');
}