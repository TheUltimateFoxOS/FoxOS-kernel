#include <driver/mouse.h>

using namespace driver;

//#MouseEventHandler::MouseEventHandler-doc: Empty constructor.
MouseEventHandler::MouseEventHandler() {

}

//#MouseEventHandler::OnMouseDown-doc: Virtual function to be overridden. Gets called when a mouse button is pressed.
void MouseEventHandler::OnMouseDown(uint8_t button) {

}

//#MouseEventHandler::OnMouseDown-doc: Virtual function to be overridden. Gets called when the mouse moves.
void MouseEventHandler::OnMouseMove(uint8_t mouse_packet[4]) {

}

//#MouseDriver::MouseDriver-doc: Mouse driver constructor.
MouseDriver::MouseDriver(MouseEventHandler* handler) : interrupts::InterruptHandler(0x2C), dataport(0x60), commandport(0x64) {
	this->handler = handler;
}


//#MouseDriver::MouseWait-doc: Wait for mouse to be ready for the next command.
void MouseDriver::MouseWait() {
	uint32_t timeout = 1000;
	while (timeout--){
		if ((commandport.Read() & 0b10) == 0){
			return;
		}
	}
}

//#MouseDriver::MouseWaitInput-doc: Wait for mouse input.
void MouseDriver::MouseWaitInput() {
	uint32_t timeout = 1000;
	while (timeout--){
		if (commandport.Read() & 0b1){
			return;
		}
	}
}

//#MouseDriver::MouseWrite-doc: Write to the mouse IO port.
void MouseDriver::MouseWrite(uint8_t value) {
	MouseWait();
	commandport.Write(0xD4);
	MouseWait();
	dataport.Write(value);
}

//#MouseDriver::MouseRead-doc: Read from the mouse IO port.
uint8_t MouseDriver::MouseRead() {
	MouseWaitInput();
	return dataport.Read();
}

//#MouseDriver::activate-doc: Activate the mouse driver.
void MouseDriver::activate() {
	commandport.Write(0xa8);
	MouseWait();
	commandport.Write(0x20);
	MouseWaitInput();
	uint8_t status = dataport.Read();
	status |= 0b10;
	MouseWait();
	commandport.Write(0x60);
	MouseWait();
	dataport.Write(status);
	MouseWrite(0xf6);
	MouseRead();
	MouseWrite(0xf4);
	MouseRead();
}

//#MouseDriver::is_presend-doc: Get if is pre send.
bool MouseDriver::is_presend() {
	return true;
}

//#MouseDriver::handle-doc: Mouse event interrupt handler.
void MouseDriver::handle() {
	uint8_t data = MouseRead();
	static bool skip = true;
	if (skip) {
		skip = false;
		return;
	}
	
	switch(mouse_cycle) {
		case 0:
			if ((data & 0b00001000) == 0)
				break;
			mouse_packet[0] = data;
			mouse_cycle++;
			break;
		case 1:
			mouse_packet[1] = data;
			mouse_cycle++;
			break;
		case 2:
			mouse_packet[2] = data;
			mouse_packet_ready = true;
			mouse_cycle = 0;
			break;
	}

	if(!mouse_packet_ready)
		return;

	if(handler != 0) {
		handler->OnMouseMove(mouse_packet);

		if(mouse_packet[0] & 1) {
			handler->OnMouseDown(LeftButton);
		}
		
		if((mouse_packet[0] >> 1) & 1) {
			handler->OnMouseDown(RightButton);
		}
		
		if((mouse_packet[0] >> 2) & 1) {
			handler->OnMouseDown(MiddleButton);
		}

	}

	mouse_packet_ready = false;
}

//#MouseDriver::get_name-doc: Get the driver name.
char* MouseDriver::get_name() {
	return (char*) "mouse";
}