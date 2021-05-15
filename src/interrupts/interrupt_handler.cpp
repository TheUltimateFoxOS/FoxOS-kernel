#include <interrupts/interrupt_handler.h>

using namespace interrupts;
InterruptHandler* interrupts::handlers[256];
intr_handler_ptr interrupts::static_handlers[256];

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

void interrupts::register_interrupt_handler(uint8_t intr, intr_handler_ptr handler) {
	static_handlers[intr] = handler;
}