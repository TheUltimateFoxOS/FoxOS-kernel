#pragma once

#include <stdarg.h>
#include <stdint.h>

#include <renderer/framebuffer.h>
#include <renderer/point.h>
#include <renderer/font.h>

#include <memory/memory.h>

namespace renderer {
	class FontRenderer {
		public:
			FontRenderer(framebuffer_t* target_frame_buffer, psf1_font_t* font);
			renderer::point_t cursor_position;
			framebuffer_t* target_frame_buffer;
			psf1_font_t* font;
			uint32_t color;

			void vsprintf(const char* str, va_list ap);
			void printf(const char* str, ...);
			void putc(char c);
			void puts(const char* s);
			void putn(unsigned long x, int base);
			void clear();
			void clear_line();
	};

	extern FontRenderer* global_font_renderer;
}