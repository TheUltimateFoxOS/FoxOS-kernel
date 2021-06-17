#include <fs/fat32/ff.h>

#include <renderer/font_renderer.h>

#include <paging/page_frame_allocator.h>

void fat32_test() {
	FATFS fs;
	FIL fp;
	UINT btr, br;
	FRESULT fr;

	f_mount(&fs, "", 0);

	fr = f_open(&fp, "/startup.nsh", FA_READ);
	if (fr == FR_OK) {
		btr = f_size(&fp);
		void* file_contents = (uint8_t*) global_allocator.request_pages(btr / 0x1000 + 1);
		f_read(&fp, file_contents, btr, &br);

		renderer::global_font_renderer->printf("%s", file_contents);

		fr = f_close(&fp);
	}
}