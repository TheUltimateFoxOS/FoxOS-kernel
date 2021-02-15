#include <interrupts/interrupt_handler.h>

using namespace interrupts;

InterruptHandler* interrupts::handlers[256];

InterruptHandler::InterruptHandler(uint8_t int_num) {
	this->int_num = int_num;
	handlers[this->int_num] = this;
}

InterruptHandler::~InterruptHandler() {
	if(handlers[this->int_num] == this) {
		handlers[this->int_num] = NULL;
	}
}

void InterruptHandler::handle() {

}