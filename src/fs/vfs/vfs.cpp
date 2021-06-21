#include <fs/vfs/vfs.h>
#include <scheduling/scheduler/errno.h>
#include <memory/heap.h>
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
	char _filename[1024];
	char* file_path = NULL;

	strcpy(_filename, filename);

	int len = strlen(_filename);

	for (int i = 0; i < len; i++) {
		if(_filename[i] == ':') {
			_filename[i] = 0;
			file_path = (char*) ((uint64_t) &_filename[i] + 1);
			goto found;
		}
	}

	//#warning The node trying to be accessed can not be found
	set_task_errno(vfs_result::VFS_NO_NDOE);

	return NULL;

found:
	list_node_t* node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
	if (node == NULL) {
		//#warning The node trying to be accessed is null
		set_task_errno(vfs_result::VFS_NO_NDOE);
		return NULL;
	}
	if (((vfs_mount*) node->data1)->open == NULL) {
		//#warning Set an errno: A VFS driver function is missing
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return NULL;
	}
	FILE* out = ((vfs_mount*) node->data1)->open((vfs_mount*) node->data1, file_path, mode);	

	out->mount = (vfs_mount*) node->data1;

	return out;
}

int fclose(FILE* stream) {
	if (stream->mount->close == NULL) {
		//#warning Set an errno: A VFS driver function is missing
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 1;
	}
	stream->mount->close(stream->mount, stream);
	free(stream);

	return 0;
}

size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	if (stream->mount->write == NULL) {
		//#warning Set an errno: A VFS driver function is missing
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 0;
	}
	size_t ret = stream->mount->write(stream->mount, ptr, size, nmemb, stream);
	return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	if (stream->mount->read == NULL) {
		//#warning Set an errno: A VFS driver function is missing
		set_task_errno(vfs_result::VFS_MISSING_FUNCTION);
		return 0;
	}
	size_t ret = stream->mount->read(stream->mount, ptr, size, nmemb, stream);
	return ret;
}