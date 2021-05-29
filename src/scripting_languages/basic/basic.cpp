#include <scripting_languages/basic/basic.h>
#include <driver/serial.h>
#include <paging/page_frame_allocator.h>

void BASIC_error(const char* error, int line_num) {
	driver::global_serial_driver->printf("Error line %d: %s", line_num, error);
}

void parse_buffer(char* buffer, token_type_t type, int line_num) {
	if (type == token_type_t::error) {
		BASIC_error("Unknown error occured", line_num);
	} else if (type == token_type_t::function) {
		driver::global_serial_driver->printf("FUNCTION\n");
	} else if (type == token_type_t::string) {
		driver::global_serial_driver->printf("STRING\n");
	} else if (type == token_type_t::label) {
		driver::global_serial_driver->printf("LABEL\n");
	}
}

void eval_BASIC(const char* code) {
	char* buffer = (char*)global_allocator.request_page();
	int buffer_len = 0;
	int line_num = 1;

	int state = 0;
	token_type_t last_token_type = token_type_t::function;

	while(*code) {
		char str = *code++;

		switch(str) {
			case '\n':
				if (state == 1) {
					BASIC_error("End of string not found!", line_num);
					global_allocator.free_page(buffer);
					return;
				}

				parse_buffer(buffer, last_token_type, line_num);

				memset(buffer, 0, 4096);
				last_token_type = token_type_t::function;
				buffer_len = 0;
				line_num++;
				break;
			case ' ':
				if (state == 0) {
					parse_buffer(buffer, last_token_type, line_num);

					memset(buffer, 0, 4096);
					last_token_type = token_type_t::function;
					buffer_len = 0;
				} else {
					buffer[buffer_len] = str;
					buffer_len++;
				}
				break;
			case '\"':
				if (state == 1) {
					state = 0;
					last_token_type = token_type_t::string;
				} else {
					state = 1;
				}
				break;
			case ':':
				if (state == 1) {
					buffer[buffer_len] = str;
					buffer_len++;
					break;
				}

				last_token_type = token_type_t::label;
				break;
			default:
				buffer[buffer_len] = str;
				buffer_len++;
				break;
		}
	}
	if (buffer_len != 0) {
		parse_buffer(buffer, last_token_type, line_num);
	}

	global_allocator.free_page(buffer);
}
