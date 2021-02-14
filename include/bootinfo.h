#ifndef BOOTINFO_H
#define BOOTINFO_H

#include <renderer/font.h>
#include <renderer/framebuffer.h>

struct bootinfo_t {
	psf1_font_t* font;
	framebuffer_t* framebuffer;
};

#endif