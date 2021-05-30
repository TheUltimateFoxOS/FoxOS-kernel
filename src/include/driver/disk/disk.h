#ifndef DISK_H
#define DISK_H

#include <stdint.h>

#define DISK_NUM 265

namespace driver {
	namespace disk {
		class Disk {
			public:
				Disk();
				~Disk();
				virtual void read(uint64_t sector, uint32_t sector_count, void* buffer);
				virtual void write(uint64_t sector, uint32_t sector_count, void* buffer);
		};

		class DiskManager {
			private:
				Disk* disks[DISK_NUM];
				int num_disks;
			public:
				DiskManager();

				void read(int disk_num, uint64_t sector, uint32_t sector_count, void* buffer);
				void write(int disk_num, uint64_t sector, uint32_t sector_count, void* buffer);

				void add_disk(Disk* disk);
		};

		extern DiskManager* global_disk_manager;
	}
}

#endif