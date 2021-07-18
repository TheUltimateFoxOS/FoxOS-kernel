#pragma once

#include <stivale.h>
#include <fs/vfs/vfs.h>

vfs_mount* initialise_stivale_modules(stivale_struct* bootinfo);

FILE* stivale_modules_open(vfs_mount* node, const char* file, const char* mode);
int stivale_modules_close(vfs_mount* node, file_t* stream);

size_t stivale_modules_read(vfs_mount*, void* buffer, size_t size, size_t nmemb, file_t* stream);

void stivale_modules_mount(vfs_mount* node);
