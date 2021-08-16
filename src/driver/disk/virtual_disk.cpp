#include <driver/disk/virtual_disk.h>

using namespace driver;

VirtualDisk::VirtualDisk(disk::Disk* disk, uint64_t lba_offset) {
	this->disk = disk;
	this->lba_offset = lba_offset;
}

VirtualDisk::~VirtualDisk() {
}

void VirtualDisk::read(uint64_t sector, uint32_t sector_count, void* buffer) {
	uint64_t lba = sector + lba_offset;
	disk->read(lba, sector_count, buffer);
}

void VirtualDisk::write(uint64_t sector, uint32_t sector_count, void* buffer) {
	uint64_t lba = sector + lba_offset;
	disk->write(lba, sector_count, buffer);
}