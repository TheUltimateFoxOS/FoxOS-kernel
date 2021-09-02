#include <renderer/renderer2D.h>

#include <string.h>

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

void Renderer2D::scroll_down() {
	global_renderer2D->clear_mouse_cursor(global_mouse_renderer->get_mouse_pointer(), global_mouse_renderer->mouse_position);
	/*for (unsigned long y = 0; y <= 16; y++) { // clear the first line
		for (unsigned long x = 0; x <= target_frame_buffer->width; x++) {
			*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = 0x00000000;
		}
	}
	for (unsigned long y = 16; y <= target_frame_buffer->height - 1; y++) { // copy the rest
		for (unsigned long x = 0; x <= target_frame_buffer->width; x++) {
			uint32_t old = *(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4));
			*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + ((y - 16) * target_frame_buffer->pixels_per_scanline * 4)) = old;
		}
	}
	for (unsigned long y = target_frame_buffer->height - 16; y <= target_frame_buffer->height - 1; y++) { // clear the last line
		for (unsigned long x = 0; x <= target_frame_buffer->width; x++) {
			*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = 0x00000000;
		}
	}*/

	memset(target_frame_buffer->base_address, 0, target_frame_buffer->width * 16 * 4);
	memcpy(target_frame_buffer->base_address, (void*) ((uint64_t) target_frame_buffer->base_address + target_frame_buffer->width * 16 * 4), target_frame_buffer->buffer_size - target_frame_buffer->width * 16 * 4);

	for (unsigned long y = target_frame_buffer->height - 16; y <= target_frame_buffer->height - 1; y++) { // clear the last line
		for (unsigned long x = 0; x <= target_frame_buffer->width; x++) {
			*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = 0x00000000;
		}
	}

	global_renderer2D->draw_overlay_mouse_cursor(global_mouse_renderer->get_mouse_pointer(), global_mouse_renderer->mouse_position, 0xffffffff);
}

void Renderer2D::load_bitmap(uint8_t data[], int y) {
	uint8_t info[54];
	int _i = 54;
	int _y = 0;
	uint8_t* read_buff = data;
	while(_i > 0) {
		uint8_t g = *read_buff;
		info[_y] = g;
		read_buff++;
		_y++;
		_i--;
	}
	int data_offset = *(int*)&info[10]; 
	int src_width = *(int*)&info[18];
	int src_height = *(int*)&info[22];
	int width = src_width * src_height / src_height;
	int bit_count = (*(short*)&info[28]) / 8;

	int lx = (target_frame_buffer->width - src_width) / 2;
	int ly = y;

	int location = (lx + ly * target_frame_buffer->width) * 4;

	uint8_t* logo_data = data;
	logo_data += data_offset;

	for (int i = src_height; 0 < i; i--) {
		for(int j = 0; j < width; j++) {
			int where = (j + (i * target_frame_buffer->width)) * 4 + location;     
			for (int c = 2; 0 <= c; c--) {
				uint8_t g = logo_data[((j * src_width) / width + (((src_height - i) * src_height) / src_height) * src_width) * bit_count + c];
				uint8_t* screen = (uint8_t*) target_frame_buffer->base_address;
				screen[where + c] = g;
			}
		}
	}

	renderer::global_font_renderer->cursor_position = { 0, src_height + 16 };
}

void Renderer2D::load_bitmap(uint8_t data[], int x, int y) {
	uint8_t info[54];
	int _i = 54;
	int _y = 0;
	uint8_t* read_buff = data;
	while(_i > 0) {
		uint8_t g = *read_buff;
		info[_y] = g;
		read_buff++;
		_y++;
		_i--;
	}
	int data_offset = *(int*)&info[10]; 
	int src_width = *(int*)&info[18];
	int src_height = *(int*)&info[22];
	int width = src_width * src_height / src_height;
	int bit_count = (*(short*)&info[28]) / 8;

	int lx = x;
	int ly = y;

	int location = (lx + ly * target_frame_buffer->width) * 4;

	uint8_t* logo_data = data;
	logo_data += data_offset;

	for (int i = src_height; 0 < i; i--) {
		for(int j = 0; j < width; j++) {
			int where = (j + (i * target_frame_buffer->width)) * 4 + location;     
			for (int c = 2; 0 <= c; c--) {
				uint8_t g = logo_data[((j * src_width) / width + (((src_height - i) * src_height) / src_height) * src_width) * bit_count + c];
				uint8_t* screen = (uint8_t*) target_frame_buffer->base_address;
				screen[where + c] = g;
			}
		}
	}

	//renderer::global_font_renderer->cursor_position = { 0, src_height + 16 };
}

renderer::point_t Renderer2D::get_bitmap_info(uint8_t data[]) {
	uint8_t info[54];
	int _i = 54;
	int _y = 0;
	uint8_t* read_buff = data;
	while(_i > 0) {
		uint8_t g = *read_buff;
		info[_y] = g;
		read_buff++;
		_y++;
		_i--;
	}
	int src_width = *(int*)&info[18];
	int src_height = *(int*)&info[22];

	return { src_height, src_width };
}
