#pragma once

#include <stdarg.h>
#include <stdint.h>

#include <renderer/renderer2D.h>
#include <renderer/framebuffer.h>
#include <renderer/font_renderer.h>
#include <renderer/point.h>

#include <memory/memory.h>

namespace renderer {
	class MouseRenderer {
		private:
			uint8_t* mouse_pointer;

		public:
			MouseRenderer();

			renderer::point_t mouse_position;
			renderer::point_t mouse_position_old;

			uint8_t* get_mouse_pointer();
			void on_mouse_down(uint8_t button);
			void on_mouse_move(uint8_t mouse_packet[4]);
	};

	extern MouseRenderer* global_mouse_renderer;
}