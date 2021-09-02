#pragma once

namespace shell {
	class Shell {
		private:
			char* command_buffer;
			int buffer_len;

			bool command_running;
		public:
			Shell();

			void keypress(char c);
	};

	extern Shell* global_shell;
}