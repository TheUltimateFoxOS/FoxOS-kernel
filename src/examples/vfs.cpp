#include <fs/vfs/vfs.h>
#include <fs/fat32/vfs.h>

#include <paging/page_frame_allocator.h>

#include <renderer/font_renderer.h>

/* ------------------------------------------------------------
---- DO NOT TRY TO INITIALISE A DISK TWICE AT THE SAME ID! ----
------------------------------------------------------------ */

//#vfs_test-doc: Test the virtual file system. DO NOT TRY TO INITIALISE A DISK TWICE AT THE SAME ID!
void vfs_test() {
    vfs_mount* fat_mount = initialise_fat32(0); //Initialise the FAT32 disk at ID 0
	mount(fat_mount, (char*) "vfstest"); //Mount the FAT32 disk at vfstest:/

	FILE* file = fopen("vfstest:/startup.nsh", "r"); //Open file
	int file_size = file->size; //Get the file's size
	void* buffer = global_allocator.request_pages(file_size / 0x1000 + 1); //Request pages to store the file's contents in
	
    fread(buffer, file_size, 1, file); //Read the file contents into the buffer
    renderer::global_font_renderer->printf("%s", buffer); //Print the file's contents from the buffer

	fclose(file); //Close the file
	global_allocator.free_pages(buffer, file_size / 0x1000 + 1); //Free the pages requested for the buffer
}