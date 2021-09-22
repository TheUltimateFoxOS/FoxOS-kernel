#pragma once

#include <net/listv2.h>
#include <fs/vfs/vfs.h>

namespace fd {
	class FileDescriptor {
		public:
			int fd;

			FileDescriptor(int fd);
			~FileDescriptor();

			void close();
			void open(const char* file, const char* mode);
			void read(void* buffer, size_t size, size_t nmemb);
			void write(void* buffer, size_t size, size_t nmemb);

			file_t* file;
	};

	class FileDescriptorManager {
		public:
			FileDescriptorManager();
			~FileDescriptorManager();

			listv2<FileDescriptor*> fds;

			int curr_fd = 0;

			int alloc_fd();
			void free_fd(int fd);
			void register_fd(FileDescriptor* fd_obj);
			FileDescriptor* get_fd(int fd);
	};

	extern FileDescriptorManager* fdm;
}