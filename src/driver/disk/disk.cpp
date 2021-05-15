#include <driver/disk/disk.h>

using namespace driver;
using namespace driver::disk;

DiskManager* driver::disk::global_disk_manager;

Disk::Disk() {

}

Disk::~Disk() {

}

void Disk::read(uint64_t sector, uint32_t sector_count, void* buffer) {

}

void Disk::write(uint64_t sector, uint32_t sector_count, void* buffer) {

}



DiskManager::DiskManager() {
	this->num_disks = 0;
}

void DiskManager::add_disk(Disk* disk) {
	this->disks[this->num_disks] = disk;
	renderer::global_font_renderer->printf("Adding new disk at idx %d!\n", this->num_disks);
	this->num_disks++;
}

void DiskManager::read(int disk_num, uint64_t sector, uint32_t sector_count, void* buffer) {
	this->disks[disk_num]->read(sector, sector_count, buffer);
}

void DiskManager::write(int disk_num, uint64_t sector, uint32_t sector_count, void* buffer) {
	this->disks[disk_num]->write(sector, sector_count, buffer);
}