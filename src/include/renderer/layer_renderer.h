#pragma once

#include <renderer/framebuffer.h>

namespace renderer {

	typedef framebuffer_t layer_t;

	layer_t* allocate_layer_matching(framebuffer_t* target_frame_buffer);
	void destroy_layer(layer_t* layer);

	class layer_renderer {
		public:
			layer_renderer(framebuffer_t* target_frame_buffer);
			~layer_renderer();

			void* buffer;

			void render();
			void render_layer(layer_t* layer, bool base_layer);

			framebuffer_t* target_frame_buffer;
	};

}