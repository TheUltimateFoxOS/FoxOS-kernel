#include <driver/disk/virtual_disk.h>

using namespace driver;

//#VirtualDisk::VirtualDisk-doc: This class is used to split a physical disk into multiple virtual disks using a sector offset. Currently used to split a disk into multiple partitions.
VirtualDisk::VirtualDisk(disk::Disk* disk, uint64_t lba_offset) {
	this->disk = disk;
	this->lba_offset = lba_offset;
}

//#VirtualDisk::~VirtualDisk-doc: Empty destructor.
VirtualDisk::~VirtualDisk() {
}

//#VirtualDisk::read-doc: This function is used to override the default function from the Drive base class.
void VirtualDisk::read(uint64_t sector, uint32_t sector_count, void* buffer) {
	uint64_t lba = sector + lba_offset;
	disk->read(lba, sector_count, buffer);
}

//#VirtualDisk::write-doc: This function is used to override the default function from the Drive base class.
void VirtualDisk::write(uint64_t sector, uint32_t sector_count, void* buffer) {
	uint64_t lba = sector + lba_offset;
	disk->write(lba, sector_count, buffer);
}