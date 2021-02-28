#include <renderer/mouse_renderer.h>

using namespace renderer;

MouseRenderer* renderer::global_mouse_renderer;

void MouseRenderer::on_mouse_down(uint8_t button) {
	if (button == 1) { //Left
		global_renderer2D->put_pix(global_mouse_utils->mouse_position.x, global_mouse_utils->mouse_position.y, 0xff00ff00);
	} else if (button == 2) { //Right
		global_renderer2D->put_pix(global_mouse_utils->mouse_position.x, global_mouse_utils->mouse_position.y, 0xff0000ff);
	} else if (button == 3) { //Middle
		global_renderer2D->put_pix(global_mouse_utils->mouse_position.x, global_mouse_utils->mouse_position.y, 0xffff0000);
	}
}

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
		global_mouse_utils->mouse_position.x += mouse_packet[1];
		if (xOverflow){
			global_mouse_utils->mouse_position.x += 255;
		}
	} else {
		mouse_packet[1] = 256 - mouse_packet[1];
		global_mouse_utils->mouse_position.x -= mouse_packet[1];
		if (xOverflow){
			global_mouse_utils->mouse_position.x -= 255;
		}
	}

	if (!yNegative) {
		global_mouse_utils->mouse_position.y -= mouse_packet[2];
		if (yOverflow){
			global_mouse_utils->mouse_position.y -= 255;
		}
	} else {
		mouse_packet[2] = 256 - mouse_packet[2];
		global_mouse_utils->mouse_position.y += mouse_packet[2];
		if (yOverflow){
			global_mouse_utils->mouse_position.y += 255;
		}
	}

	if (global_mouse_utils->mouse_position.x < 0) global_mouse_utils->mouse_position.x = 0;
	if (global_mouse_utils->mouse_position.x > global_renderer2D->target_frame_buffer->width-1) global_mouse_utils->mouse_position.x = global_renderer2D->target_frame_buffer->width-1;
	
	if (global_mouse_utils->mouse_position.y < 0) global_mouse_utils->mouse_position.y = 0;
	if (global_mouse_utils->mouse_position.y > global_renderer2D->target_frame_buffer->height-1) global_mouse_utils->mouse_position.y = global_renderer2D->target_frame_buffer->height-1;

	global_renderer2D->clear_mouse_cursor(global_mouse_utils->get_mouse_pointer(), global_mouse_utils->mouse_position_old);
	global_renderer2D->draw_overlay_mouse_cursor(global_mouse_utils->get_mouse_pointer(), global_mouse_utils->mouse_position, 0xffffffff);

	global_mouse_utils->mouse_position_old = global_mouse_utils->mouse_position;
}