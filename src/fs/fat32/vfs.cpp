#include <fs/fat32/vfs.h>
#include <fs/fat32/ff.h>
#include <memory/memory.h>
#include <memory/heap.h>
#include <paging/page_frame_allocator.h>
#include <string.h>
#include <assert.h>

vfs_mount* initialise_fat32(int disk_id) {
	vfs_mount* mount = new vfs_mount;
	memset(mount, 0, sizeof(mount));

	mount->data2 = disk_id;
	mount->mount = fat32_mount;
	mount->unmount = fat32_unmount;
	mount->open = fat32_open;
	mount->close = fat32_close;
	mount->write = fat32_write;
	mount->read = fat32_read;

	return mount;
}


void fat32_mount(vfs_mount* node) {
	FATFS* fs = (FATFS*) global_allocator.request_page(); // dont ask why but fatfs doesent like heap addresses

	f_mount(fs, "", node->data2);

	node->data = fs;
}

void fat32_unmount(vfs_mount* node) {
	global_allocator.free_page(node->data);
}

FILE* fat32_open(vfs_mount* node, const char* file, const char* mode) {
	FIL fil;
	BYTE fatmode;
	FILE* fp = new FILE;

	if (strcmp((char*)mode, (char*)"r") == 0) {
		fatmode = FA_READ;
		fp->is_readable = 1;
		fp->is_writable = 0;
	} else if (strcmp((char*)mode, (char*)"w") == 0) {
		fatmode = FA_WRITE;
		fp->is_readable = 0;
		fp->is_writable = 1;
	} else if (strcmp((char*)mode, (char*)"a") == 0) {
		fatmode = FA_OPEN_APPEND;
		fp->is_readable = 0;
		fp->is_writable = 1;
	} else if (strcmp((char*)mode, (char*)"r+") == 0) {
		fatmode = FA_OPEN_ALWAYS | FA_WRITE | FA_READ;
		fp->is_readable = 1;
		fp->is_writable = 1;
	} else if (strcmp((char*)mode, (char*)"w+") == 0) {
		fatmode = FA_CREATE_ALWAYS | FA_WRITE | FA_READ;
		fp->is_readable = 1;
		fp->is_writable = 1;
	} else if (strcmp((char*)mode, (char*)"a+") == 0) {
		fatmode = FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_READ;
		fp->is_readable = 1;
		fp->is_writable = 1;
	}

	FRESULT fr = f_open(&fil, file, fatmode);
	assert(fr == FR_OK);

	fp->size = f_size(&fil);
	fp->data = (void*) malloc(sizeof(FIL));
	memcpy(fp->data, &fil, sizeof(FIL));

	return fp;
}

int fat32_close(vfs_mount* node, file_t* stream) {
	f_close((FIL*) stream->data);
	free(stream->data);

	return 0;
}

size_t fat32_read(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream) {
	unsigned int has_read;
	FRESULT res = f_read((FIL*)stream->data, buffer, size, &has_read);
	assert(res == FR_OK);
	return has_read;
}

size_t fat32_write(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream) {
	unsigned int has_written;
	FRESULT res = f_write((FIL*)stream->data, buffer, size, &has_written);
	assert(res == FR_OK);
	return has_written;
}