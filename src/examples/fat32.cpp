#include <fs/fat32.h>
#include <paging/page_frame_allocator.h>

void fat32_test() {
	fat32::disk_id = 0; // set to first disk
	uint8_t fs_buf[512];
	fat32::fs_info_t fs_info = fat32::read_info(fs_buf); // read fs info
	show_info(fs_info); // print fs info to serial console



	fat32::sector_buffer_t sector_buffer;
	fat32::file_info_t fp;
	fat32::fopen("/STARTUP.NSH", "r", &fp, fs_info, &sector_buffer); // open file

	uint8_t* b = (uint8_t*) global_allocator.request_page();

	fat32::fread(b, 4096, &fp, fs_info, &sector_buffer); // read file
	driver::global_serial_driver->printf("%s", b);

	global_allocator.free_page(b);
}