#include <renderer/renderer2D.h>

#include <string.h>

using namespace renderer;

Renderer2D* renderer::global_renderer2D;

//#Renderer2D::Renderer2D-doc: Renderer2D constructor.
Renderer2D::Renderer2D(framebuffer_t* target_frame_buffer) {
	this->target_frame_buffer = target_frame_buffer;
}

//#Renderer2D::put_pix-doc: Set a pixel on screen.
void Renderer2D::put_pix(uint32_t x, uint32_t y, uint32_t colour){
	*(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4)) = colour;
}

//#Renderer2D::get_pix-doc: Get a pixel on screen.
uint32_t Renderer2D::get_pix(uint32_t x, uint32_t y){
	return *(uint32_t*)((uint64_t)target_frame_buffer->base_address + (x*4) + (y * target_frame_buffer->pixels_per_scanline * 4));
}

//#Renderer2D::clear_mouse_cursor-doc: Clear the mouse cursor.
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

//#Renderer2D::draw_overlay_mouse_cursor-doc: Draw the mouse cursor on screen and store what is under it.
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

//#Renderer2D::scroll_down-doc: Scroll the screen down.
void Renderer2D::scroll_down() {
	global_renderer2D->clear_mouse_cursor(global_mouse_renderer->get_mouse_pointer(), global_mouse_renderer->mouse_position);

	memcpy((void*) (uint64_t)this->target_frame_buffer->base_address, (void*)((uint64_t)this->target_frame_buffer->base_address + (16 * this->target_frame_buffer->pixels_per_scanline * 4)), (this->target_frame_buffer->width * 4 * (this->target_frame_buffer->height - 16)));
	memset((void*) ((uint64_t)this->target_frame_buffer->base_address + ((this->target_frame_buffer->width * 4) * (this->target_frame_buffer->height - 16))), 0, (this->target_frame_buffer->width * 4 * 16));
	memset((void*) ((uint64_t)this->target_frame_buffer->base_address + ((this->target_frame_buffer->width * 4) * (this->target_frame_buffer->height - 16))), 0, (this->target_frame_buffer->width * 4 * 16));

	// global_renderer2D->draw_overlay_mouse_cursor(global_mouse_renderer->get_mouse_pointer(), global_mouse_renderer->mouse_position, 0xffffffff);
}

//#Renderer2D::load_bitmap-doc: Load a bitmap into the center of the frame buffer at a Y position OR at an X and a Y position.
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

//#Renderer2D::get_bitmap_info-doc: Get a bitmap's information.
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
