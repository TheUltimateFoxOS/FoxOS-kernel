#include <fs/vfs/vfs.h>

#include <memory/heap.h>
#include <assert.h>
#include <string.h>

list* vfs_nodes = NULL;

void mount(vfs_mount* mount, char* name) {
	if(vfs_nodes == NULL) {
		vfs_nodes = new list(255);
	}

	char* _name = (char*) malloc(sizeof(char) * strlen(name));
	strcpy(_name, name);

	assert(mount->mount != NULL);

	list_node_t* node = vfs_nodes->add_node(mount, _name, NULL, NULL);

	mount->node = node;

	mount->mount(mount);

}

bool search_for_disk_name(list_node_t* node, void* d1, void* d2, void* d3, void* d4) {
	if(strcmp((char*) node->data2, (char*) d2) == 0) {
		return true;
	} else {
		return false;
	}
}

void unmount(char* name) {
	list_node_t* node = vfs_nodes->find_node(search_for_disk_name, NULL, name, NULL, NULL);
	assert(node != NULL);
	assert(((vfs_mount*) node->data1)->unmount != NULL);

	((vfs_mount*) node->data1)->unmount((vfs_mount*) node->data1);

	vfs_nodes->remove_node(node);
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

	return NULL;

found:
	list_node_t* node = vfs_nodes->find_node(search_for_disk_name, NULL, _filename, NULL, NULL);
	assert(node != NULL);
	assert(((vfs_mount*) node->data1)->open != NULL);
	FILE* out = ((vfs_mount*) node->data1)->open((vfs_mount*) node->data1, file_path, mode);	

	out->mount = (vfs_mount*) node->data1;

	return out;
}

int fclose(FILE* stream) {
	assert(stream->mount->close != NULL);
	stream->mount->close(stream->mount, stream);
	free(stream);

	return 0;
}

size_t fwrite(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	assert(stream->mount->write != NULL);
	size_t ret = stream->mount->write(stream->mount, ptr, size, nmemb, stream);
	return ret;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
	assert(stream->mount->read != NULL);
	size_t ret = stream->mount->read(stream->mount, ptr, size, nmemb, stream);
	return ret;
}