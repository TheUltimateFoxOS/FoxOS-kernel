#pragma once

namespace shell {
	class Shell {
		private:
			char* command_buffer;
			int buffer_len;

			bool command_running;
		public:
			Shell();

			void init_shell();
			void print_nl();
			void run_command();

			void keypress(char c);
	};

	extern Shell* global_shell;
}