#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>

struct framebuffer_t {
	void* base_addres;
	size_t buffer_size;
	uint32_t width;
	uint32_t height;
	uint32_t pixels_per_scanline;
};

#endif