#include <renderer/font_renderer.h>

using namespace renderer;

FontRenderer* renderer::global_font_renderer;

FontRenderer::FontRenderer(framebuffer_t* target_frame_buffer, psf1_font_t* font) {
	this->target_frame_buffer = target_frame_buffer;
	this->font = font;

	color = 0xffffffff;
	cursor_position = {0, 0};
}

void FontRenderer::printf(const char* str, ...) {
	va_list ap;
	const char* s;
	unsigned long n;

	va_start(ap, str);

	while(*str) {
		if(*str == '%') {
			str++;
			switch(*str) {
				case 's':
					s = va_arg(ap, char*);
					this->puts(s);
					break;
				case 'c':
					n = va_arg(ap, int);
					this->putc(n);
					break;
				case 'd':
				case 'u':
					n = va_arg(ap, unsigned long int);
					this->putn(n, 10);
					break;
				case 'x':
				case 'p':
					n = va_arg(ap, unsigned long int);
					this->putn(n, 16);
					break;
				case 'f':
					color = va_arg(ap, uint32_t);
					break;
				case 'r':
					color = 0xffffffff;
					break;
				case '%':
					this->putc('%');
					break;
				case '\0':
					goto out;
					break;
				default:
					this->putc('%');
					this->putc(*str);
					break;
			}
		} else {
			this->putc(*str);
		}
		str++;
	}

out:
	va_end(ap);
}

void FontRenderer::putc(char c) {

	if(c == 0) {
		return;
	}

	if(c == '\b') {
		uint32_t* pix_ptr = (uint32_t*) target_frame_buffer->base_address;
		for (unsigned long y = cursor_position.y; y < cursor_position.y + 16; y++){
			for (unsigned long x = cursor_position.x; x < cursor_position.x + 8; x++){
				*(unsigned int*)(pix_ptr + x + (y * target_frame_buffer->pixels_per_scanline)) = 0;
			}
		}

		if (cursor_position.x - 16 < 0) {
			cursor_position.x = target_frame_buffer->width - 8;
			if (cursor_position.y - 16 < 0) {
				cursor_position.y = 0;
			} else {
				cursor_position.y -= 16;
			}
		} else {
			cursor_position.x -= 8;
		}
		return;
	}

	if(cursor_position.x + 16 > target_frame_buffer->width || c == '\n') {
		cursor_position.x = 0;
		cursor_position.y += 16;
	} else {
		cursor_position.x += 8;
	}

	if (cursor_position.y + 16 > target_frame_buffer->height) {
		global_renderer2D->scroll_down();
		cursor_position.y -= 16;
	}

	if(c == '\n') {
		return;
	}

	uint32_t* pix_ptr = (uint32_t*) target_frame_buffer->base_address;
	char* font_ptr = (char*) font->glyph_buffer + (c * font->psf1_Header->charsize);

	for (unsigned long y = cursor_position.y; y < cursor_position.y + 16; y++){
		for (unsigned long x = cursor_position.x; x < cursor_position.x + 8; x++){
			if ((*font_ptr & (0b10000000 >> (x - cursor_position.x))) > 0){
					*(unsigned int*)(pix_ptr + x + (y * target_frame_buffer->pixels_per_scanline)) = color;
				}
		}
		font_ptr++;
	}
}

void FontRenderer::puts(const char* s) {
	while(*s) {
		this->putc(*s++);
	}
}

void FontRenderer::putn(unsigned long x, int base) {
	char buf[65];
	const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";

	char* p;

	if(base > 36) {
		return;
	}

	p = buf + 64;
	*p = '\0';

	do {
		*--p = digits[x % base];
		x /= base;
	} while(x);

	this->puts(p);
}

void FontRenderer::clear() {
	uint64_t base = (uint64_t) target_frame_buffer->base_address;
    uint64_t bytes_per_scanline = target_frame_buffer->pixels_per_scanline * 4;
    uint64_t fb_height = target_frame_buffer->height;
    uint64_t fb_size = target_frame_buffer->buffer_size;

    for (int vertical_scanline = 0; vertical_scanline < fb_height; vertical_scanline ++){
        uint64_t pix_ptr_base = base + (bytes_per_scanline * vertical_scanline);
        for (uint32_t* pixPtr = (uint32_t*)pix_ptr_base; pixPtr < (uint32_t*)(pix_ptr_base + bytes_per_scanline); pixPtr ++){
            *pixPtr = color;
        }
    }
}