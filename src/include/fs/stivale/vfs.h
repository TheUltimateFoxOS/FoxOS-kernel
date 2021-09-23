#pragma once

#include <stivale2.h>
#include <fs/vfs/vfs.h>

vfs_mount* initialise_stivale_modules(stivale2_struct* bootinfo);

FILE* stivale_modules_open(vfs_mount* node, const char* file, const char* mode);
int stivale_modules_close(vfs_mount* node, file_t* stream);

int stivale_modules_seek(vfs_mount*, file_t* file, long int offset, int whence);
long int stivale_modules_tell(vfs_mount*, file_t* file);

size_t stivale_modules_read(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);

void stivale_modules_mount(vfs_mount* node);
