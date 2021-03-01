#pragma once

#include <renderer/renderer2D.h>
#include <renderer/framebuffer.h>
#include <renderer/font_renderer.h>
#include <renderer/point.h>
#include <renderer/mouse_utils.h>

#include <driver/mouse.h>
#include <stdarg.h>
#include <stdint.h>
#include <memory/memory.h>

namespace renderer {
	class MouseRenderer {
		public:
			void on_mouse_down(uint8_t button);
			void on_mouse_move(uint8_t mouse_packet[4]);
	};

	extern MouseRenderer* global_mouse_renderer;
}