#include <fs/stivale/vfs.h>

#include <driver/serial.h>

#include <memory/memory.h>

#include <scheduling/scheduler/errno.h>

//#initialise_stivale_modules-doc: Initialise a new mount point for a stivale module mount.
vfs_mount* initialise_stivale_modules(stivale2_struct* bootinfo) {
	vfs_mount* mount = new vfs_mount;
	memset(mount, 0, sizeof(mount));

	stivale2_struct_tag_modules* modules = stivale2_tag_find<stivale2_struct_tag_modules>(bootinfo, STIVALE2_STRUCT_TAG_MODULES_ID);

	mount->data = (void*) modules->modules;
	mount->data2 = modules->module_count;

	mount->open = stivale_modules_open;
	mount->read = stivale_modules_read;
	mount->close = stivale_modules_close;

	mount->mount = stivale_modules_mount;

	return mount;
}

//#stivale_modules_mount-doc: The mount function for a stivale mount point.
void stivale_modules_mount(vfs_mount* node) {
	stivale2_module* mod = (stivale2_module*) node->data;

	for (int i = 0; i < node->data2; i++) {
		driver::global_serial_driver->printf("Found stivale module %s at 0x%x ending at 0x%x!\n", mod[i].string, mod[i].begin, mod[i].end);
	}

}

//#stivale_modules_open-doc: The open function for a stivale mount point.
FILE* stivale_modules_open(vfs_mount* node, const char* file, const char* mode) {
	FILE* fp = new FILE;

	if (strcmp((char*)mode, (char*)"r") == 0) {
		fp->is_readable = 1;
		fp->is_writable = 0;
	} else if (strcmp((char*)mode, (char*)"w") == 0) {
		set_task_errno(VFS_NOT_IMPLEMENTED);
		fp->is_error = 1;
		return fp;
	} else if (strcmp((char*)mode, (char*)"a") == 0) {
		set_task_errno(VFS_NOT_IMPLEMENTED);
		fp->is_error = 1;
		return fp;
	} else if (strcmp((char*)mode, (char*)"r+") == 0) {
		set_task_errno(VFS_NOT_IMPLEMENTED);
		fp->is_error = 1;
		return fp;
	} else if (strcmp((char*)mode, (char*)"w+") == 0) {
		set_task_errno(VFS_NOT_IMPLEMENTED);
		fp->is_error = 1;
		return fp;
	} else if (strcmp((char*)mode, (char*)"a+") == 0) {
		set_task_errno(VFS_NOT_IMPLEMENTED);
		fp->is_error = 1;
		return fp;
	}

	stivale2_module* mod = (stivale2_module*) node->data;

	for (int i = 0; i < node->data2; i++) {
		if(strcmp(mod[i].string, (char*) file) == 0) {
			fp->size = mod[i].end - mod[i].begin;
			fp->data = (void*) mod[i].begin;

			goto found;
		}
	}

	set_task_errno(VFS_FILE_NOT_FOUND);
	fp->is_error = 1;
	return fp;

found:
	return fp;
}

//#stivale_modules_close-doc: The close function for a stivale mount point.
int stivale_modules_close(vfs_mount* node, file_t* stream) {
	stream->data = NULL;
	stream->size = 0;
	stream->is_readable = 0;

	return 0;
}

//#stivale_modules_read-doc: The read function for a stivale mount point.
size_t stivale_modules_read(vfs_mount* node, void* buffer, size_t size, size_t nmemb, file_t* stream) {
	for (int i = 0; i < size; i++) {
		*(uint8_t*) ((uint64_t) buffer + i) = *((uint8_t*) stream->data + i);
	}
	
	return size;
}