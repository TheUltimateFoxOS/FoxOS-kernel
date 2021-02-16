#include <renderer/renderer2D.h>

using namespace renderer;

Renderer2D* renderer::global_renderer2D;

Renderer2D::Renderer2D(framebuffer_t* target_frame_buffer) {
	this->target_frame_buffer = target_frame_buffer;
}

void Renderer2D::put_pix(uint32_t x, uint32_t y, uint32_t colour){
	*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = colour;
}

uint32_t Renderer2D::get_pix(uint32_t x, uint32_t y){
	return *(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4));
}

void Renderer2D::clear_mouse_cursor(uint8_t* mouse_pointer, point_t pos){
	if (!mouse_drawn) return;

	int xMax = 16;
	int yMax = 16;
	int differenceX = target_frame_buffer->width - pos.x;
	int differenceY = target_frame_buffer->height - pos.y;

	if (differenceX < 16) xMax = differenceX;
	if (differenceY < 16) yMax = differenceY;

	for (int y = 0; y < yMax; y++){
		for (int x = 0; x < xMax; x++){
			int bit = y * 16 + x;
			int byte = bit / 8;
			if ((mouse_pointer[byte] & (0b10000000 >> (x % 8))))
			{
				if (get_pix(pos.x + x, pos.y + y) == mouse_cursor_buffer_after[x + y *16]) {
					put_pix(pos.x + x, pos.y + y, mouse_cursor_buffer[x + y * 16]);
				}
			}
		}
	}
}

void Renderer2D::draw_overlay_mouse_cursor(uint8_t* mouse_pointer, point_t pos, uint32_t colour) {
	int xMax = 16;
	int yMax = 16;
	int differenceX = target_frame_buffer->width - pos.x;
	int differenceY = target_frame_buffer->height - pos.y;

	if (differenceX < 16) xMax = differenceX;
	if (differenceY < 16) yMax = differenceY;

	for (int y = 0; y < yMax; y++) {
		for (int x = 0; x < xMax; x++) {
			int bit = y * 16 + x;
			int byte = bit / 8;
			if ((mouse_pointer[byte] & (0b10000000 >> (x % 8))))
			{
				mouse_cursor_buffer[x + y * 16] = get_pix(pos.x + x, pos.y + y);
				put_pix(pos.x + x, pos.y + y, colour);
				mouse_cursor_buffer_after[x + y * 16] = get_pix(pos.x + x, pos.y + y);
			}
		}
	}

	mouse_drawn = true;
}