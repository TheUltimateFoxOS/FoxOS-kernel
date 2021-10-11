#include <stivale2.h>
#include <config.h>

static unsigned char stack[0x4000];

#ifdef USE_STIVALE2_SMP
static stivale2_header_tag_smp smp_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_SMP_ID,
		.next = 0,
	},
	.flags = 1 // use x2APIC if aviable
};
#endif

static stivale2_header_tag_framebuffer framebuffer_tag = {
	.tag = {
		.identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
	#ifdef USE_STIVALE2_SMP
		.next = (uint64_t) &smp_tag,
	#else
		.next = 0,
	#endif
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

stivale2_struct* global_bootinfo;
