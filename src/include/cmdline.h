#pragma once

#include <fs/vfs/list.h>

typedef void (*cmdline_handler)(char*);

class CmdLineParser: public list {
	public:
		CmdLineParser();
		~CmdLineParser();

		void add_handler(char* name, cmdline_handler handler);
		void parse(char* cmdline);
};
