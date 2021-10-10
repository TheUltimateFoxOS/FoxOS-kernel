#include <fs/fat32/fat32_old.h>

#include <driver/serial.h>

#include <paging/page_frame_allocator.h>

//#fat32_old_test-doc: Test the OLD fat32 driver.
void fat32_old_test(char* _) {
	fat32_old::disk_id = 0; //Set to the first disk
	uint8_t fs_buf[512];
	fat32_old::fs_info_t fs_info = fat32_old::read_info(fs_buf); //Read file system info
	show_info(fs_info); //Print the info to the serial console


	fat32_old::sector_buffer_t sector_buffer;
	fat32_old::file_info_t fp;
	fat32_old::fopen("/STARTUP.NSH", "r", &fp, fs_info, &sector_buffer); //Open file

	uint8_t* buffer = (uint8_t*) global_allocator.request_page(); //Request a page to store data

	fat32_old::fread(buffer, 4096, &fp, fs_info, &sector_buffer); //Read 4096 bytes from the file into the buffer
	fat32_old::fclose(&fp, &sector_buffer);

	driver::global_serial_driver->printf("%s", buffer); //Print the file's contents onto the serial console

	global_allocator.free_page(buffer); //Free the requested page
}
