#pragma once

#include <renderer/framebuffer.h>
#include <renderer/font.h>
#include <stdarg.h>
#include <stdint.h>

namespace renderer {

	struct point_t {
		uint32_t x;
		uint32_t y;
	};

	class FontRenderer {
		public:
			FontRenderer(framebuffer_t* target_frame_buffer, psf1_font_t* font);
			renderer::point_t cursor_position;
			framebuffer_t* target_frame_buffer;
			psf1_font_t* font;
			uint32_t color;

			void printf(const char* str, ...);
			void putc(char c);
			void puts(const char* s);
			void putn(unsigned long x, int base);
	};
}