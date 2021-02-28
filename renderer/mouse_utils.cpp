#include <renderer/mouse_utils.h>

using namespace renderer;

MouseUtils* renderer::global_mouse_utils;

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

MouseUtils::MouseUtils() {
	this->mouse_pointer = MousePointer;
}

uint8_t* MouseUtils::get_mouse_pointer() {
	return this->mouse_pointer;
}