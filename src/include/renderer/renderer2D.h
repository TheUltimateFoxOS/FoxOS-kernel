#pragma once

#include <stdarg.h>
#include <stdint.h>

#include <renderer/framebuffer.h>
#include <renderer/point.h>
#include <renderer/mouse_renderer.h>

#include <memory/memory.h>

namespace renderer {
	class Renderer2D {
		public:
			Renderer2D(framebuffer_t* target_frame_buffer);
			framebuffer_t* target_frame_buffer;

			void put_pix(uint32_t x, uint32_t y, uint32_t colour);
			uint32_t get_pix(uint32_t x, uint32_t y);


			uint32_t mouse_cursor_buffer[16 * 16];
			uint32_t mouse_cursor_buffer_after[16 * 16];

			void clear_mouse_cursor(uint8_t* mouse_pointer, renderer::point_t pos);
			void draw_overlay_mouse_cursor(uint8_t* mouse_pointer, renderer::point_t pos, uint32_t colour);
			void load_bitmap(uint8_t data[], int y);
			void load_bitmap(uint8_t data[], int x, int y);
			renderer::point_t get_bitmap_info(uint8_t data[]);
			void scroll_down();
			bool mouse_drawn;
	};

	extern Renderer2D* global_renderer2D;
}