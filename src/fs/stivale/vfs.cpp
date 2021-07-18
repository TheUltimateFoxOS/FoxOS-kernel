#include <fs/stivale/vfs.h>

#include <driver/serial.h>

#include <memory/memory.h>

#include <scheduling/scheduler/errno.h>


vfs_mount* initialise_stivale_modules(stivale_struct* bootinfo) {
	vfs_mount* mount = new vfs_mount;
	memset(mount, 0, sizeof(mount));

	mount->data = (void*) bootinfo->modules;
	mount->data2 = bootinfo->module_count;

	mount->open = stivale_modules_open;
	mount->read = stivale_modules_read;
	mount->close = stivale_modules_close;

	mount->mount = stivale_modules_mount;

	return mount;
}

void stivale_modules_mount(vfs_mount* node) {
	stivale_module* mod = (stivale_module*) node->data;

	for (int i = 0; i < node->data2; i++) {
		driver::global_serial_driver->printf("Found stivale module %s at 0x%x ending at 0x%x!\n", mod->string, mod->begin, mod->end);
		mod = (stivale_module*) mod->next;
	}

}

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

	stivale_module* mod = (stivale_module*) node->data;

	for (int i = 0; i < node->data2; i++) {
		if(strcmp(mod->string, (char*) file) == 0) {
			fp->size = mod->end - mod->begin;
			fp->data = (void*) mod->begin;

			goto found;
		}
		mod = (stivale_module*) mod->next;
	}

	set_task_errno(VFS_FILE_NOT_FOUND);
	fp->is_error = 1;
	return fp;

found:
	return fp;
}

int stivale_modules_close(vfs_mount* node, file_t* stream) {
	stream->data = NULL;
	stream->size = 0;
	stream->is_readable = 0;

	return 0;
}

size_t stivale_modules_read(vfs_mount* node, void* buffer, size_t size, size_t nmemb, file_t* stream) {
	for (int i = 0; i < size; i++) {
		*(uint8_t*) ((uint64_t) buffer + i) = *((uint8_t*) stream->data + i);
	}
	
	return size;
}