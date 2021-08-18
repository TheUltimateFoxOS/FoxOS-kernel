#include <stivale2.h>

static unsigned char stack[0x4000];

static stivale2_header_tag_framebuffer framebuffer_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
		.next = 0,
	},
	.framebuffer_width = 0,
	.framebuffer_height = 0,
	.framebuffer_bpp = 32
};

__attribute__((section(".stivale2hdr"), used))
static stivale2_header __stivale2_header = {
	.entry_point = 0,
	.stack = (uintptr_t) stack + sizeof(stack),
	.flags = 0,
	.tags = (uint64_t) &framebuffer_tag
};