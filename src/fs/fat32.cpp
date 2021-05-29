#include <fs/fat32.h>

using namespace fat32;

int fat32::disk_id = 0;
uint32_t fat32::fs_start_sector;

int fat32::read_sector(uint8_t* buffer, uint32_t sector) {
	driver::disk::global_disk_manager->read(disk_id, sector, 1, buffer);
	return 0;
}

int fat32::write_sector(uint8_t* buffer, uint32_t sector) {
	driver::disk::global_disk_manager->write(disk_id, sector, 1, buffer);
	return 0;
}

int fat32::fs_read_sector(uint8_t* buffer, uint32_t sector) {
	return read_sector(buffer,sector + fs_start_sector);
}

MBR_info_t fat32::MBR_read(uint8_t* buffer) {
	read_sector(buffer, 0);
	MBR_t *MBR;
	MBR = (MBR_t *)buffer;
	MBR_info_t ret;
	ret.active = (MBR->MBR_partition_table->MBR_boot_indicator) != 0;
	strcpy(ret.fs_type, (MBR->MBR_partition_table->MBR_partion_type == 0x0c) ? "FAT32" : "UNKNOWN");
	ret.reserved_sector = MBR->MBR_partition_table->MBR_sectors_preceding;
	fs_start_sector = ret.reserved_sector;
	return ret;
}

fs_info_t fat32::BPB_read(uint8_t* buffer) {
	fs_read_sector(buffer, 0);
	BPB_t *BPB;
	BPB = (BPB_t *)buffer;

	if (BPB->bytes_per_sector != 512) {
		driver::global_serial_driver->printf("BPB_read: Wrong sector size %d\n", BPB->bytes_per_sector);
	}

	fs_info_t fs_info;
	strncpy(fs_info.label,(char*) BPB->BS_volume_label, 11);
	fs_info.fat_size = BPB->fat_size;
	fs_info.root_dir_sectors = 0;
	fs_info.total_sectors =BPB->total_sectors_32;
	fs_info.data_sectors = fs_info.total_sectors - (BPB->reserved_sector_count + (BPB->num_fats * fs_info.fat_size) + fs_info.root_dir_sectors);
	fs_info.sectors_per_cluster = BPB->sectors_per_cluster;

	if(fs_info.sectors_per_cluster != 0) {
		fs_info.data_clusters = fs_info.data_sectors / fs_info.sectors_per_cluster;
	}

	fs_info.reserved_sectors = BPB->reserved_sector_count;
	fs_info.data_start_sector = BPB->reserved_sector_count + (BPB->num_fats * fs_info.fat_size) + fs_info.root_dir_sectors;
	return fs_info;
}

fs_info_t fat32::read_info(uint8_t* buffer) {
	MBR_info_t mbr_info;
	mbr_info.active = 0;
	strcpy(mbr_info.fs_type, "FAT32");
	mbr_info.reserved_sector = 0;
	mbr_info.start_cylinder = 0;
	mbr_info.start_head = 0;
	mbr_info.start_head = 0;
	mbr_info.start_sector = 0;
	fs_start_sector = 0;
	auto fs_info = BPB_read(buffer);
	return fs_info;
}

int fat32::fsync(sector_buffer_t* buffer) {
	return write_sector(buffer->data, fs_start_sector+buffer->sector_number);
}

int fat32::fetch(uint32_t sector, sector_buffer_t* buffer) {
	uint32_t error_level = 0;
	if (buffer->sector_number == sector) {
		error_level |= SECTOR_ALREADY_IN_MEMORY;
		return error_level;
	}

	if (buffer->sector_flags & CURRENT_FLAG_DIRTY) {
		error_level |= fsync(buffer);
	}

	error_level |= fs_read_sector(buffer->data, sector);
	buffer->sector_number = sector;
	return error_level;
}

uint32_t fat32::get_fat_entry(uint32_t cluster, fs_info_t fs_info, sector_buffer_t* buffer) {
	uint32_t offset = 4 * cluster;
	uint32_t sector = fs_info.reserved_sectors + (offset/512);
	fetch(sector, buffer);
	uint32_t* FAT_entry = ((uint32_t*) &(buffer->data[offset % 512]));
	return* FAT_entry;
}

static char formated_file_name[16];
char* fat32::format_file_name(directory_entry_t *entry) {
	int i, j;
	uint8_t* entryname = entry->filename;
	if (entry->attributes != 0x0f) {
		j = 0;
		for (i = 0; i < 8; i++) {
			if (entryname[i] != ' ') {
				formated_file_name[j++] = entryname[i];
			}
		}

		if(entryname[8] != ' ') {
			formated_file_name[j++] = '.';
		}

		for (i = 8; i < 11; i++) {
			if (entryname[i] != ' ') {
				formated_file_name[j++] = entryname[i];
			}
		}

		formated_file_name[j++] = '\x00';
	} else {
		long_file_name_t* LongEntry = (long_file_name_t*) entry;
		j = 0;
		for (i = 0; i < 5; i++) {
			formated_file_name[j++] = (uint8_t) LongEntry->name_1[i];
		}
		for (i = 0; i < 6; i++) {
			formated_file_name[j++] = (uint8_t) LongEntry->name_2[i];
		}
		for (i = 0; i < 2; i++) {
			formated_file_name[j++] = (uint8_t) LongEntry->name_3[i];
		}
		formated_file_name[j++] = '\x00';
	}

	return (char*) formated_file_name;
}

file_info_t fat32::read_one_file_info(directory_entry_t* dir_entry, uint32_t cluster, fs_info_t fs_info) {
	file_info_t ret;
	if ((dir_entry->attributes & ATTR_LONG_NAME_MASK) == ATTR_LONG_NAME) {
		long_file_name_t* LongFileName = (long_file_name_t*) dir_entry;
		strcpy(ret.long_filename, format_file_name(dir_entry));
		ret.attributes = ATTR_LONG_NAME;
		ret.checksum = LongFileName->checksum;
	} else {
		strcpy(ret.filename, "");
		strcpy(ret.filename, format_file_name(dir_entry));
		ret.attributes = dir_entry->attributes;
		ret.parent_start_cluster = cluster;
		ret.current_byte = 0;
		ret.start_cluster = ret.current_cluster = (((uint32_t)dir_entry->cluster_low) << 16) | dir_entry->first_cluster;
		ret.flags = 0;
		ret.mode = ENRTRY_MODE_WRITE;
		ret.pos = 0;
		ret.file_size = dir_entry->file_size;
		if ((ret.attributes & ATTR_DIRECTORY) == 1) {
			ret.file_size = SECTOR_SIZE*fs_info.sectors_per_cluster;
		}
		ret.current_cluster_offset = 0;
	}
	return ret;
}

file_info_t* fat32::find_file(uint32_t cluster, const char *filename, file_info_t *fp, fs_info_t fs_info, sector_buffer_t *buffer, int recursive) {
	if(recursive <= 0) {
		return nullptr;
	}

	uint32_t FAT_content = get_fat_entry(cluster, fs_info, buffer);

	fs_read_sector(buffer->data, (uint32_t)((fs_info.data_start_sector+ cluster - 2)));
	size_t offset = 0;
	char LongFileNameBuffer[LONG_FILENAME_MAX_LEN];
	LongFileNameBuffer[0] = 0;

	for (; offset < fs_info.sectors_per_cluster*SECTOR_SIZE; offset += 0x20) {
		directory_entry_t* CurrentOneFileInfo = (directory_entry_t *)&buffer->data[offset];
		auto file_info=read_one_file_info(CurrentOneFileInfo, cluster, fs_info);
		if ((file_info.attributes & 0x0f) == 0x0f) {
			//part of long filename
			strcpy(LongFileNameBuffer + strlen(LongFileNameBuffer), file_info.long_filename);
		} else {
			strcpy(file_info.long_filename,LongFileNameBuffer);
			//Got one file
			LongFileNameBuffer[0] = 0;
			if(file_info.filename[0] == '\000' || file_info.filename[0] == 0x7f) {
				//file is deleted
				continue;
			} else {
				if(strncmp(filename, file_info.filename, strlen((char*) filename)) == 0 || strncmp(filename, file_info.long_filename, strlen((char*) filename)) == 0) {
					*fp = file_info;
					return fp;
				} else {
					driver::global_serial_driver->printf("find_file: %s is not %s\n", filename, file_info.filename);
				}
			}
		}
	}
	return nullptr;
}

file_info_t* fat32::fopen(const char *filename, const char *mode, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer) {

	if(fp == nullptr) {
		return nullptr;
	}

	uint32_t cluster = 0;
	size_t n = strlen((char*) filename);
	
	if(n < 1 || filename[0] != '/') {
		return nullptr;
	}
	
	cluster = 2;
	fp->current_cluster = 2;
	fp->start_cluster = 2;
	fp->parent_start_cluster = 0xffffffff;
	fp->current_cluster_offset = 0;
	fp->current_sector = 0;
	fp->current_byte = 0;
	fp->attributes = ATTR_DIRECTORY;
	fp->pos = 0;
	fp->flags |= ENRTRY_FLAG_ROOT;
	fp->file_size = 0xffffffff;
	fp->mode = ENRTRY_MODE_READ | ENRTRY_MODE_WRITE | ENRTRY_MODE_OVERWRITE;
	size_t i = 0, j = 0;

	while(true) {
		if(!filename[i] || filename[i] == '/') {
			if(i - j > 1) {
				char part_name[LONG_FILENAME_MAX_LEN];
				size_t len = i - j - 1;
				strncpy(part_name, filename + j + 1, len);
				part_name[len] = 0;
				fp = find_file(cluster, part_name, fp, fs_info, buffer, 10);

				if(fp == nullptr) {
					break;
				}

				cluster = fp->start_cluster;
			}
			j = i;
		}
		if(!filename[i]) {
			break;
		}
		i++;
	}
	return fp;
}

uint32_t fat32::find_free_cluster(uint32_t base, fs_info_t fs_info, sector_buffer_t* buffer) {
	uint32_t cluster = base;
	uint32_t TotalCluster = fs_info.total_sectors / fs_info.sectors_per_cluster;

	for (; cluster <= TotalCluster; cluster++) {
		if ((get_fat_entry(cluster, fs_info, buffer) & 0x0fffffff) == 0) {
			return cluster;
		}
	}
	cluster = 0;
	for (; cluster < base; cluster++) {
		if ((get_fat_entry(cluster, fs_info, buffer) & 0x0fffffff) == 0) {
			return cluster;
		}
	}
	return ERROR_VOLUME_FULL;
}

int fat32::set_fat_entry(uint32_t cluster, uint32_t value, fs_info_t fs_info, sector_buffer_t* buffer) {
	uint32_t error_level = 0;
	uint32_t offset = 4 * cluster;
	uint32_t sector = fs_info.reserved_sectors + (offset / 512);
	error_level |= fetch(sector, buffer);
	uint32_t *FAT_entry = ((uint32_t *) &(buffer->data[offset % 512]));
	if (*FAT_entry != value) {
		buffer->sector_flags &= CURRENT_FLAG_DIRTY;
		*FAT_entry = value;
	}
	return error_level;
}

int fat32::fseek(file_info_t* fp, int32_t base, long offset, fs_info_t fs_info, sector_buffer_t* buffer) {
	long pos = base + offset;
	uint32_t cluster_offset;
	uint32_t temp;
	if ((pos > (long)fp->file_size) && (fp->attributes!= ATTR_DIRECTORY)) {
		return ERROR_SEEK_FILE;
	} else if ((long)fp->file_size == pos) {
		fp->flags |= ENRTRY_FLAG_SIZECHANGED;
		fp->file_size += 1;
	} else {
		cluster_offset = pos / (fs_info.sectors_per_cluster * 512);
		if (cluster_offset != fp->current_cluster_offset) {
			temp = cluster_offset;
			if (cluster_offset > fp->current_cluster_offset) {
				cluster_offset -= fp->current_cluster_offset;
			} else {
				fp->current_cluster = fp->start_cluster;
			}
			fp->current_cluster_offset = temp;
			while (cluster_offset > 0) {
				temp = get_fat_entry(fp->current_cluster, fs_info, buffer);
				if ((temp & FAT_MASK_EOC) != FAT_MASK_EOC) {
					fp->current_cluster = temp;
				} else {
					temp = find_free_cluster(fp->current_cluster, fs_info, buffer);
					set_fat_entry(fp->current_cluster, temp, fs_info, buffer);
					set_fat_entry(temp, FAT_MASK_EOC, fs_info, buffer);
					fp->current_cluster = temp;
				}
				cluster_offset--;
				if (fp->current_cluster >= FAT_MASK_EOC) {
					if (cluster_offset > 0) {
						return ERROR_SEEK_FILE;
					}
				}
			}
		}
		fp->current_byte = (pos % (fs_info.sectors_per_cluster * 512));
		fp->pos = pos;
	}

	return 0;
}

uint32_t fat32::cluster_to_sector(uint32_t cluster, fs_info_t fs_info) {
	uint32_t base = fs_info.data_start_sector;
	uint32_t offset = (cluster - 2)*fs_info.sectors_per_cluster;
	return base + offset ;
}

int fat32::fread(uint8_t* dest, size_t size, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer) {
	uint32_t sector;
	for (; size > 0; size--) {
		sector = (fp->current_byte / 512) + cluster_to_sector(fp->current_cluster, fs_info);
		fetch(sector, buffer);
		*dest++ = buffer->data[fp->current_byte % 512];
		if (fp->attributes & ATTR_DIRECTORY) {
			if (fseek(fp, 0, fp->pos + 1, fs_info, buffer)) {
				return ERROR_SEEK_FILE;
			}
		} else {
			if (fseek(fp, 0, fp->pos + 1, fs_info, buffer)) {
				return ERROR_SEEK_FILE;
			}
		}
	}
	return 0;
}

int fat32::fwrite(uint8_t* src, uint32_t size, uint32_t count, file_info_t* fp, fs_info_t fs_info, sector_buffer_t* buffer) {
	uint32_t i, tracking, segsize;
	fp->flags |= ENRTRY_FLAG_DIRTY;
	while (count > 0) {
		i = size;
		fp->flags |= ENRTRY_FLAG_SIZECHANGED;
		while (i > 0) {
			uint32_t dest_sector = cluster_to_sector(fp->current_cluster, fs_info) + (fp->current_byte / 512);
			fetch(dest_sector, buffer);
			tracking = fp->current_byte % 512;
			segsize = (i<512 ? i : 512);
			memcpy(&buffer->data[tracking], src, segsize);
			buffer->sector_flags |= ENRTRY_FLAG_DIRTY;
			if (fp->pos + segsize > fp->file_size) {
				fp->file_size += segsize - (fp->pos % 512);
			}
			if (fseek(fp, 0, fp->pos + segsize, fs_info, buffer)) {
				return -1;
			}
			i -= segsize;
			src += segsize;
		}
		count--;
	}
	return size - i;
}

int fat32::ffsync(file_info_t* fp, sector_buffer_t* buffer) {
	if (buffer->sector_flags & CURRENT_FLAG_DIRTY) {
		fsync(buffer);
	}

	fp->flags &= ~ENRTRY_FLAG_DIRTY;
	fp->flags &= ~ENRTRY_FLAG_SIZECHANGED;
	return 0;
}

int fat32::fclose(file_info_t* fp, sector_buffer_t* buffer) {
	ffsync(fp,buffer);
	return 0;
}

int fat32::touch(const char* filename, fs_info_t fs_info, sector_buffer_t* buffer) {
	directory_entry_t entry;
	uint32_t cluster;
	const char *name;
	const char *ext;
	name = strrchr(filename, '/');
	if(name == nullptr) {
		return -1;
	}
	name = name + 1;
	ext = strrchr(filename, '.');
	size_t ext_len;
	size_t name_len;
	if(ext == nullptr) {
		ext_len = 0;
		name_len = strlen((char*) name);
	} else {
		ext = ext + 1;
		ext_len = strlen((char*) ext);
		name_len = strlen((char*) name) - ext_len - 1;
	}

	char parent[strlen((char*) filename)];
	strcpy(parent, filename);
	parent[name-filename] = 0;
	file_info_t f;
	file_info_t *fp = fopen(parent, "r+", &f, fs_info, buffer);
	if(!fp) {
		return 1;
	}

	// Now we have the directory in which we want to create the file, open for overwrite

	do {
		//seek to the end
		fread((uint8_t*)&entry, sizeof(directory_entry_t), fp, fs_info, buffer);
	} while(entry.filename[0] != '\x00');

	// Back up one entry, this is where we put the new filename entry
	fseek(fp, (int32_t) -sizeof(directory_entry_t), fp->pos, fs_info, buffer);
	cluster = find_free_cluster(0, fs_info, buffer);
	set_fat_entry(cluster, FAT_MASK_EOC, fs_info, buffer); // Marks the new cluster as the last one (but no longer free)

	entry.attributes = ATTR_ARCHIVE;
	entry.reserved = 0;
	entry.creation_time_ms = 0x6F;
	entry.creation_time = 0xAE70;
	entry.creation_date = 0x4CD5;
	entry.last_access_time = 0x4CD5;
	entry.cluster_low = (cluster>>16) & 0xffff;
	entry.modified_time = 0xAE71;
	entry.modified_date = 0x4CD5;
	entry.first_cluster = cluster & 0xffff;
	entry.file_size = 0;
	strncpy((char*)entry.filename, name, name_len < 8 ? name_len : 8);
	strncpy((char*)entry.extension, ext ? ext : "   ", 3);

	for(int i = 0; i < 8; ++i) {
		if(entry.filename[i] == 0) {
			entry.filename[i] = ' ';
		}
	}
	
	for(int i = 0; i < 3; ++i) {
		if(entry.extension[i] == 0) {
			entry.extension[i] = ' ';
		}
	}

	fwrite((uint8_t*)&entry, sizeof(directory_entry_t), 1, fp, fs_info, buffer);
	memset(&entry, 0, sizeof(directory_entry_t));
	entry.filename[0] = '\x00';
	fwrite((uint8_t*)&entry, sizeof(directory_entry_t), 1, fp, fs_info, buffer);
	fclose(fp, buffer);
	return 0;
}

void fat32::show_info(fs_info_t fs_info) {
	driver::global_serial_driver->printf("-----------------------------FS INFO-------------------------------\n");
	driver::global_serial_driver->printf("This Volume has a total of %u sectors!\n", fs_info.total_sectors);
	driver::global_serial_driver->printf("Cluster Size:%d\n", fs_info.sectors_per_cluster*SECTOR_SIZE);
	driver::global_serial_driver->printf("label: ");

	for (int i = 0; i < 11; i++) {
		driver::global_serial_driver->printf("%c", fs_info.label[i]);
	}

	driver::global_serial_driver->printf("\n");
	driver::global_serial_driver->printf("Reserved Sectors: %u!\n", fs_info.reserved_sectors);
	driver::global_serial_driver->printf("Fat zone has a total of %u sectors!\n", fs_info.fat_size);
	driver::global_serial_driver->printf("Data zone Start from Cluster: %u!\n", fs_info.data_start_sector);
	driver::global_serial_driver->printf("Data zone has a total of %u sectors!\n", fs_info.data_sectors);
	driver::global_serial_driver->printf("Data zone has a total of %u clusters!\n", fs_info.data_clusters);
	driver::global_serial_driver->printf("-----------------------------FS INFO END---------------------------\n");
}