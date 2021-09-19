#include <interrupts/interrupt_handler.h>
#include <stddef.h>

using namespace interrupts;
InterruptHandler* interrupts::handlers[256];
intr_handler_ptr interrupts::static_handlers[256];

//#InterruptHandler::InterruptHandler-doc: Registers the interrupt handler for the given interrupt number. This is called by is the constructor.
InterruptHandler::InterruptHandler(uint8_t int_num) {
	this->int_num = int_num;
	handlers[this->int_num] = this;
}

//#InterruptHandler::~InterruptHandler-doc: Unregisters the interrupt handler for the given interrupt number. This is a destructor no need to call it manually.
InterruptHandler::~InterruptHandler() {
	if(handlers[this->int_num] == this) {
		handlers[this->int_num] = NULL;
	}
}

//#InterruptHandler::handle-doc: Default handler for interrupts. Gets called if the default handler doesn't get overwritten.
void InterruptHandler::handle() {

}

//#interrupts::register_interrupt_handler-doc: Register an interrupt handler for the given interrupt number. Uses the static_handlers array to store the handler.
void interrupts::register_interrupt_handler(uint8_t intr, intr_handler_ptr handler) {
	static_handlers[intr] = handler;
}