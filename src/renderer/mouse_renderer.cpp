#include <renderer/mouse_renderer.h>

#include <driver/mouse.h>

using namespace renderer;

MouseRenderer* renderer::global_mouse_renderer;

uint8_t MousePointer[] = {
	0b10000000, 0b00000000,
	0b11000000, 0b00000000,
	0b11100000, 0b00000000,
	0b11110000, 0b00000000,
	0b11111000, 0b00000000,
	0b11111100, 0b00000000,
	0b11111110, 0b00000000,
	0b11111111, 0b00000000,
	0b11111111, 0b10000000,
	0b11111111, 0b11000000,
	0b11111100, 0b00000000,
	0b11101100, 0b00000000,
	0b11000110, 0b00000000,
	0b10000110, 0b00000000,
	0b00000011, 0b00000000,
	0b00000011, 0b00000000,
};

//#MouseRenderer::MouseRenderer-doc: MouseRenderer constructor.
MouseRenderer::MouseRenderer() {
	this->mouse_pointer = MousePointer;
}

//#MouseRenderer::get_mouse_pointer-doc: Get the mouse pointer data.
uint8_t* MouseRenderer::get_mouse_pointer() {
	return this->mouse_pointer;
}

//#MouseRenderer::on_mouse_down-doc: Called when a mouse button is pressed. Currently draws dots on the screen.
void MouseRenderer::on_mouse_down(uint8_t button) {
	if (button == 1) { //Left
		global_renderer2D->put_pix(mouse_position.x, mouse_position.y, 0xff00ff00);
	} else if (button == 2) { //Right
		global_renderer2D->put_pix(mouse_position.x, mouse_position.y, 0xff0000ff);
	} else if (button == 3) { //Middle
		global_renderer2D->put_pix(mouse_position.x, mouse_position.y, 0xffff0000);
	}
}

//#MouseRenderer::on_mouse_move-doc: Called when the mouse is moved. Re-draw the cursor to a new position.
void MouseRenderer::on_mouse_move(uint8_t mouse_packet[4]) {
	bool xNegative, yNegative, xOverflow, yOverflow;

	if (mouse_packet[0] & PS2XSign) {
		xNegative = true;
	} else xNegative = false;

	if (mouse_packet[0] & PS2YSign) {
		yNegative = true;
	} else yNegative = false;

	if (mouse_packet[0] & PS2XOverflow) {
		xOverflow = true;
	} else xOverflow = false;

	if (mouse_packet[0] & PS2YOverflow) {
		yOverflow = true;
	} else yOverflow = false;

	if (!xNegative) {
		mouse_position.x += mouse_packet[1];
		if (xOverflow){
			mouse_position.x += 255;
		}
	} else {
		mouse_packet[1] = 256 - mouse_packet[1];
		mouse_position.x -= mouse_packet[1];
		if (xOverflow){
			mouse_position.x -= 255;
		}
	}

	if (!yNegative) {
		mouse_position.y -= mouse_packet[2];
		if (yOverflow){
			mouse_position.y -= 255;
		}
	} else {
		mouse_packet[2] = 256 - mouse_packet[2];
		mouse_position.y += mouse_packet[2];
		if (yOverflow){
			mouse_position.y += 255;
		}
	}

	if (mouse_position.x < 0) mouse_position.x = 0;
	if (mouse_position.x > global_renderer2D->target_frame_buffer->width-1) mouse_position.x = global_renderer2D->target_frame_buffer->width-1;
	
	if (mouse_position.y < 0) mouse_position.y = 0;
	if (mouse_position.y > global_renderer2D->target_frame_buffer->height-1) mouse_position.y = global_renderer2D->target_frame_buffer->height-1;

	global_renderer2D->clear_mouse_cursor(get_mouse_pointer(), mouse_position_old);
	global_renderer2D->draw_overlay_mouse_cursor(get_mouse_pointer(), mouse_position, 0xffffffff);

	mouse_position_old = mouse_position;
}