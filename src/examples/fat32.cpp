#include <fs/fat32/ff.h>

#include <renderer/font_renderer.h>

#include <paging/page_frame_allocator.h>

//#fat32_test-doc: Test the fat32 driver.
void fat32_test(char* _) {
	FATFS fs;
	FIL fp;
	UINT btr, br;
	FRESULT fr;

	f_mount(&fs, "", 0); //Mount the filesystem

	fr = f_open(&fp, "/startup.nsh", FA_READ); //Open  file
	if (fr == FR_OK) { //Check for errors
		btr = f_size(&fp); //Get the size of the file
		void* file_contents = global_allocator.request_pages(btr / 0x1000 + 1); //Request pages to store the file contents in
	
		f_read(&fp, file_contents, btr, &br); //Read the file contents into the buffer
		f_close(&fp); //Close the file

		renderer::global_font_renderer->printf("%s", file_contents); //Print the file's contents from the buffer

		global_allocator.free_pages(file_contents, btr / 0x1000 + 1); //Free the requested pages
	}
}
