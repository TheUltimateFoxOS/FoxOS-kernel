#include <fs/gpt/gpt.h>

#include <paging/page_frame_allocator.h>

#include <string.h>

#include <driver/serial.h>
#include <driver/disk/disk.h>
#include <driver/disk/virtual_disk.h>

using namespace gpt;

//#gpt::read_gpt-doc: Read the guid partition table from a disk. Returns false if an error occurred. Automaticaly registers a new virtual disk for every partition found.
bool gpt::read_gpt(driver::disk::Disk* disk) {
	gpt_header* header = (gpt_header*) global_allocator.request_page();
	disk->read(1, 1, (void*) header);

	if (memcmp(header->signature, "EFI PART", 8) != 0) {
		global_allocator.free_page(header);
		return false;
	} else {
		gpt_partition_entry* entries = (gpt_partition_entry*) global_allocator.request_pages(((header->partition_entries_size * header->partition_entries_count) / 4096) + 1);
		disk->read(header->partition_entries_startting_lba, ((header->partition_entries_size * header->partition_entries_count) / 512 ) + 1, (void*) entries);

		for (int i = 0; i < header->partition_entries_count; i++) {
			if (entries[i].type_guid.data1 == 0) {
				continue;
			}
			
			driver::global_serial_driver->printf("Partition guid: %x, index: %d, partition start lba: %d\n", entries[i].type_guid.data3, i, entries[i].first_lba);

			driver::VirtualDisk* vdisk = new driver::VirtualDisk(disk, entries[i].first_lba);
			driver::disk::global_disk_manager->add_disk(vdisk);
		}
		

		global_allocator.free_pages(entries, ((header->partition_entries_size * header->partition_entries_count) / 4096) + 1);
		global_allocator.free_page(header);
		return true;
	}
}