#include <cmdline.h>
#include <string.h>
#include <renderer/font_renderer.h>

//#CmdLineParser::CmdLineParser-doc: Empty constructor. Calles the constructor of list.
CmdLineParser::CmdLineParser() : list(100) {

}

//#CmdLineParser::~CmdLineParser-doc: Empty destructor.
CmdLineParser::~CmdLineParser() {

}

//#CmdLineParser::add_handler-doc: Registers a handler for a command line option.
void CmdLineParser::add_handler(char* name, cmdline_handler handler) {
	this->add_node(name, (void*) handler, 0, 0);
}

//#find_handler-doc: Helper function to find a handler for a command line argument.
bool find_handler(list_node_t* node, void* d1, void* d2, void* d3, void* d4) {
	if(strcmp((char*) node->data1, (char*) d1) == 0) {
		return true;
	} else {
		return false;
	}
}

//#CmdLineParser::parse-doc: Parses the command line and calls the appropriate handler.
void CmdLineParser::parse(char* cmdline) {
	char* last_token = cmdline;

	int len = strlen(cmdline);

	if (len == 0) {
		return;
	}

	for (int i = 0; i < len; i++) {
		if (cmdline[i] == ' ') {
			cmdline[i] = 0;

			list_node_t* node = this->find_node(find_handler, last_token, 0, 0, 0);

			if (node == 0) {
				renderer::global_font_renderer->printf("%fUnknown command: %s%r\n", 0xffff0000, last_token);
			} else {
				((cmdline_handler) node->data2)();
			}

			last_token = &cmdline[i + 1];
		}
	}
	
	list_node_t* node = this->find_node(find_handler, last_token, 0, 0, 0);

	if (node == 0) {
		renderer::global_font_renderer->printf("%fUnknown command: %s%r\n", 0xffff0000, last_token);
	} else {
		((cmdline_handler) node->data2)();
	}
}