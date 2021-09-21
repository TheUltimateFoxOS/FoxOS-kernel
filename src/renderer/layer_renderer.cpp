#include <renderer/layer_renderer.h>

#include <paging/page_frame_allocator.h>
#include <memory/heap.h>
#include <string.h>

using namespace renderer;

//#renderer::allocate_layer_matching-doc: Allocates a new layer matching the given framebuffer configuration.
layer_t* renderer::allocate_layer_matching(framebuffer_t* target_frame_buffer) {
	layer_t* layer = (layer_t*) malloc(sizeof(layer_t));

	memcpy(layer, target_frame_buffer, sizeof(layer_t));

	// allocate new frame buffer for new layer
	layer->base_address = global_allocator.request_pages((layer->buffer_size / 0x1000) + 1);

	memset(layer->base_address, 0, layer->buffer_size);

	return layer;
}


//#renderer::destroy_layer-doc: Destroys a layer.
void renderer::destroy_layer(layer_t* layer) {
	global_allocator.free_pages(layer->base_address, (layer->buffer_size / 0x1000) + 1);
	free(layer);
}

//#layer_renderer::layer_renderer-doc: Layer renderer constructor. Takes a target framebuffer.
layer_renderer::layer_renderer(framebuffer_t* target_frame_buffer) {
	this->target_frame_buffer = target_frame_buffer;

	buffer = global_allocator.request_pages((target_frame_buffer->buffer_size / 0x1000) + 1);
}

//#layer_renderer::~layer_renderer-doc: Destructor for the layer renderer.
layer_renderer::~layer_renderer() {
	global_allocator.free_pages(buffer,(target_frame_buffer->buffer_size / 0x1000) + 1);
}

extern "C" void do_copy(void* target, void* source, size_t size);

//#layer_renderer::render_layer-doc: Renders a layer to the buffer. If the layer is the first layer please set the base_layer to true.
void layer_renderer::render_layer(layer_t* layer, bool base_layer) {

	if (base_layer) {
		// copy base layer to frame buffer
		memcpy(buffer, layer->base_address, layer->buffer_size);
	} else {
		do_copy(buffer, layer->base_address, layer->buffer_size / sizeof(uint32_t));
	}
}

//#layer_renderer::render-doc: Renders the buffer to the target framebuffer.
void layer_renderer::render() {
	memcpy(target_frame_buffer->base_address, buffer, target_frame_buffer->buffer_size);
}