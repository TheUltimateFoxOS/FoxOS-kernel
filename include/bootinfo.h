#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <renderer/font.h>
#include <renderer/framebuffer.h>
#include <stdint.h>

struct bootinfo_t {
	psf1_font_t* font;
	framebuffer_t* framebuffer;
	void* m_map;
	uint64_t m_map_size;
	uint64_t m_map_desc_size;
};

#endif