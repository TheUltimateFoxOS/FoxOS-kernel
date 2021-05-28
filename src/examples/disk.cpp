#include <driver/disk/disk.h>
#include <paging/page_frame_allocator.h>
#include <driver/serial.h>

void disk_test() {
	char* buffer = (char*) global_allocator.request_page();

	driver::disk::global_disk_manager->read(0, 0, 1, buffer);

	for (int t = 0; t < 512; t++){
		driver::global_serial_driver->printf("%c", buffer[t]);
	}

	driver::global_serial_driver->printf("\n");

	global_allocator.free_page(buffer);
}