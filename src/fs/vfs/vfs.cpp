#include <fs/vfs/vfs.h>

#include <scheduling/scheduler/errno.h>

#include <memory/heap.h>

#include <paging/page_frame_allocator.h>

#include <assert.h>
#include <string.h>

list* vfs_nodes = NULL;

vfs_result mount(vfs_mount* mount, char* name) {
	if(vfs_nodes == NULL) {
		vfs_nodes = new list(255);
	}

	char* _name = (char*) malloc(sizeof(char) * strlen(name));
	strcpy(_name, name);

	if (mount->mount == NULL) {
		return VFS_MOUNT_ERROR;
	}

	list_node_t* node = vfs_nodes->add_node(mount, _name, NULL, NULL);
	mount->node = node;

	mount->mount(mount);
	return VFS_OK;
}

bool search_for_disk_name(list_node_t* node, void* d1, void* d2, void* d3, void* d4) {
	if(strcmp((char*) node->data2, (char*) d2) == 0) {
		return true;
	} else {
		return false;
	}
}

vfs_result unmount(char* name) {
	list_node_t* node = vfs_nodes->find_node(search_for_disk_name, NULL, name, NULL, NULL);
	if (node == NULL) {
		return VFS_NO_NDOE;
	}
	if (((vfs_mount*) node->data1)->unmount == NULL) {
		return VFS_MISSING_FUNCTION;
	}

	((vfs_mount*) node->data1)->unmount((vfs_mount*) node->data1);
	vfs_nodes->remove_node(node);
	return VFS_OK;
}

FILE* fopen(const char* filename, const char* mode) {
	char _filename[1024] = "";
	char* file_path = NULL;
	strcpy(_filename, filename);
	int len = strlen(_filename);
	list_node_t* node = NULL;

	for (int i = 0; i < len; i++) {
		if(_filename[i] == ':') {
			_filename[i] = 0;
			file_path = (char*) ((uint64_t) &_filename[i] + 1);
			node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
			break;
		}
	}

	if (node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return NULL;
	}
	if (((vfs_mount*) node->data1)->open == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return NULL;
	}

	FILE* out = ((vfs_mount*) node->data1)->open((vfs_mount*) node->data1, file_path, mode);	
	out->mount = (vfs_mount*) node->data1;
	return out;
}

int fclose(FILE* stream) {
	if (stream->mount->close == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 1;
	}
	stream->mount->close(stream->mount, stream);
	free(stream);

	return 0;
}

size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	if (stream->mount->write == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 0;
	}
	size_t ret = stream->mount->write(stream->mount, ptr, size, nmemb, stream);
	return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	if (stream->mount->read == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 0;
	}
	size_t ret = stream->mount->read(stream->mount, ptr, size, nmemb, stream);
	return ret;
}

DIR* opendir(const char* name) {
	char _filename[1024] = "";
	char* file_path = NULL;
	strcpy(_filename, name);
	int len = strlen(_filename);
	list_node_t* node = NULL;

	for (int i = 0; i < len; i++) {
		if(_filename[i] == ':') {
			_filename[i] = 0;
			file_path = (char*) ((uint64_t) &_filename[i] + 1);
			node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
			break;
		}
	}

	if (node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return NULL;
	}
	if (((vfs_mount*) node->data1)->open == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return NULL;
	}

	DIR* out = ((vfs_mount*) node->data1)->opendir((vfs_mount*) node->data1, file_path);
	out->mount = (vfs_mount*) node->data1;
	return out;	
}

int closedir(DIR* stream) {
	if (stream->mount->close == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 1;
	}
	stream->mount->closedir(stream->mount, stream);
	free(stream);

	return 0;
}

struct dirent* readdir(DIR* stream) {
	if (stream->mount->readdir == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 0;
	}

	dirent* ret = stream->mount->readdir(stream->mount, stream);
	return ret;
}

void rewinddir(DIR* stream) {
	if (stream->mount->rewinddir == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
	}

	stream->mount->rewinddir(stream->mount, stream);
}

int mkdir(const char* name, uint32_t mode) {
	char _filename[1024] = "";
	char* file_path = NULL;
	strcpy(_filename, name);
	int len = strlen(_filename);
	list_node_t* node = NULL;

	for (int i = 0; i < len; i++) {
		if(_filename[i] == ':') {
			_filename[i] = 0;
			file_path = (char*) ((uint64_t) &_filename[i] + 1);
			node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
			break;
		}
	}

	if (node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return -1;
	}
	if (((vfs_mount*) node->data1)->mkdir == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return -1;
	}

	return ((vfs_mount*) node->data1)->mkdir((vfs_mount*) node->data1, name, mode);
}

int unlink(const char* name) {
	char _filename[1024] = "";
	char* file_path = NULL;
	strcpy(_filename, name);
	int len = strlen(_filename);
	list_node_t* node = NULL;

	for (int i = 0; i < len; i++) {
		if(_filename[i] == ':') {
			_filename[i] = 0;
			file_path = (char*) ((uint64_t) &_filename[i] + 1);
			node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
			break;
		}
	}

	if (node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return -1;
	}
	if (((vfs_mount*) node->data1)->unlink == NULL) {
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return -1;
	}

	return ((vfs_mount*) node->data1)->unlink((vfs_mount*) node->data1, name);
}

int rename(const char* old_name, const char* new_name) {
	if (strcmp((char*) old_name, (char*) new_name) == 0) {
		return -1;
	}

	//OLD NODE
	char _old_filename[1024] = "";
	char* old_file_path = NULL;
	strcpy(_old_filename, old_name);
	int old_len = strlen(_old_filename);
	list_node_t* old_node = NULL;

	for (int i = 0; i < old_len; i++) {
		if(_old_filename[i] == ':') {
			_old_filename[i] = 0;
			old_file_path = (char*) ((uint64_t) &_old_filename[i] + 1);
			old_node = vfs_nodes->find_node(search_for_disk_name, NULL, _old_filename, NULL, NULL);
			break;
		}
	}

	if (old_node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return -1;
	}

	//NEW NODE
	char _new_filename[1024] = "";
	char* new_file_path = NULL;
	strcpy(_new_filename, new_name);
	int new_len = strlen(_new_filename);
	list_node_t* new_node = NULL;

	for (int i = 0; i < new_len; i++) {
		if(_new_filename[i] == ':') {
			_new_filename[i] = 0;
			new_file_path = (char*) ((uint64_t) &_new_filename[i] + 1);
			new_node = vfs_nodes->find_node(search_for_disk_name, NULL, _new_filename, NULL, NULL);
			break;
		}
	}

	if (new_node == NULL) {
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return -1;
	}

	if (strcmp(_old_filename, _new_filename) == 0) {
		if (((vfs_mount*) old_node->data1)->rename == NULL) {
			set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
			return -1;
		}

		return ((vfs_mount*) old_node->data1)->rename((vfs_mount*) old_node->data1, old_file_path, new_file_path);
	} else {
		#warning Renaming a file and putting it on another node has not been implemented.
		set_task_errno(vfs_result::VFS_NOT_IMPLEMENTED);
		return -1;
	}
}