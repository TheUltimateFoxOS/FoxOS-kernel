#include <fs/fd.h>
#include <scheduling/scheduler/errno.h>

#define ASSERT_NO_PANIC
#include <assert.h>

using namespace fd;

//#FileDescriptor::FileDescriptor-doc: The constructor of the FileDescriptor class.
FileDescriptor::FileDescriptor(int fd) {
	this->fd = fd;
	this->file = NULL;
}

//#FileDescriptor::~FileDescriptor-doc: Empty destructor.
FileDescriptor::~FileDescriptor() {

}

//#FileDescriptor::close-doc: Closes the file descriptor.
void FileDescriptor::close() {
	assert(this->file != NULL);

	fclose(this->file);
	return;

assert_fail:
	set_task_errno(VFS_ERROR);
}

//#FileDescriptor::open-doc: Opens the file descriptor.
void FileDescriptor::open(const char* path, const char* mode) {
	assert(this->file == NULL);

	this->file = fopen(path, mode);
	return;

assert_fail:
	set_task_errno(VFS_ERROR);
}

//#FileDescriptor::read-doc: Reads from the file descriptor.
void FileDescriptor::read(void* buffer, size_t size, size_t nmemb) {
	assert(this->file != NULL);

	fread(buffer, size, nmemb, this->file);
	return;

assert_fail:
	set_task_errno(VFS_ERROR);
}

//#FileDescriptor::write-doc: Writes to the file descriptor.
void FileDescriptor::write(void* buffer, size_t size, size_t nmemb) {
	assert(this->file != NULL);

	fwrite(buffer, size, nmemb, this->file);
	return;

assert_fail:
	set_task_errno(VFS_ERROR);
}

//#FileDescriptorManager::FileDescriptorManager-doc: The constructor of the FileDescriptorManager class.
FileDescriptorManager::FileDescriptorManager() : fds(100) {
	this->curr_fd = 10;
}

//#FileDescriptorManager::~FileDescriptorManager-doc: Empty destructor.
FileDescriptorManager::~FileDescriptorManager() {

}

//#FileDescriptorManager::alloc_fd-doc: Allocates a new file descriptor.
int FileDescriptorManager::alloc_fd() {
	return ++this->curr_fd;
}

//#FileDescriptorManager::free_fd-doc: Frees a file descriptor.
void FileDescriptorManager::free_fd(int fd) {
	listv2<FileDescriptor*>::node* n = this->fds.find<int>([](int _fd, listv2<FileDescriptor*>::node* n) {
		return _fd == n->data->fd;
	}, fd);

	assert(n != NULL);

	this->fds.remove(n);
	return;

assert_fail:
	set_task_errno(VFS_ERROR);
}

//#FileDescriptorManager::register_fd-doc: Registers a FileDescriptor object.
void FileDescriptorManager::register_fd(FileDescriptor* fd_obj) {
	this->fds.add(fd_obj);
}

//#FileDescriptorManager::get_fd-doc: Gets a FileDescriptor object by its file descriptor.
FileDescriptor* FileDescriptorManager::get_fd(int fd) {
	listv2<FileDescriptor*>::node* n = this->fds.find<int>([](int _fd, listv2<FileDescriptor*>::node* n) {
		return _fd == n->data->fd;
	}, fd);

	assert(n != NULL);
	return n->data;

assert_fail:
	set_task_errno(VFS_ERROR);
	return NULL;
}

FileDescriptorManager* fd::fdm;