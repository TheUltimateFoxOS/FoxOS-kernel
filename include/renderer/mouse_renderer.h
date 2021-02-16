#pragma once

#include <renderer/renderer2D.h>
#include <renderer/framebuffer.h>
#include <renderer/font_renderer.h>
#include <renderer/point.h>
#include <driver/mouse.h>
#include <stdarg.h>
#include <stdint.h>
#include <memory.h>

namespace renderer {
	class MouseRenderer {
		public:
			renderer::point_t mouse_position;
			renderer::point_t mouse_position_old;

			void on_mouse_down(uint8_t button);
			void on_mouse_move(uint8_t mouse_packet[4]);
	};

	extern MouseRenderer* global_mouse_renderer;
}