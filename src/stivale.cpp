#include <stivale.h>

static unsigned char stack[0x4000];

__attribute__((section(".stivalehdr"), used))
static stivale_header __stivale_header = {
	.stack = (uintptr_t) stack + sizeof(stack),
	.flags = 1 << 0,
	.framebuffer_width = 0,
	.framebuffer_height = 0,
	.framebuffer_bpp = 32,

	.entry_point = 0
};