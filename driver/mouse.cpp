#include <driver/mouse.h>

using namespace driver;

MouseEventHandler::MouseEventHandler() {

}

void MouseEventHandler::OnMouseDown(uint8_t button) {

}

void MouseEventHandler::OnMouseMove(uint8_t mouse_packet[4]) {

}

MouseDriver::MouseDriver(MouseEventHandler* handler) : interrupts::InterruptHandler(0x2C), dataport(0x60), commandport(0x64) {
	this->handler = handler;
}

void MouseDriver::MouseWait() {
    uint32_t timeout = 1000;
    while (timeout--){
        if ((commandport.Read() & 0b10) == 0){
            return;
        }
    }
}

void MouseDriver::MouseWaitInput() {
    uint32_t timeout = 1000;
    while (timeout--){
        if (commandport.Read() & 0b1){
            return;
        }
    }
}

void MouseDriver::MouseWrite(uint8_t value) {
    MouseWait();
    commandport.Write(0xD4);
    MouseWait();
    dataport.Write(value);
}

uint8_t MouseDriver::MouseRead() {
    MouseWaitInput();
    return dataport.Read();
}

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

bool MouseDriver::is_presend() {
	return true;
}

void MouseDriver::handle() {
	uint8_t data = MouseRead();
    
    switch(mouse_cycle) {
        case 0:
            if(mouse_packet_ready)
                break;
            if ((data & 0b00001000) == 0)
                break;
            mouse_packet[0] = data;
            mouse_cycle++;
            break;
        case 1:
            if(mouse_packet_ready)
                break;
            mouse_packet[1] = data;
            mouse_cycle++;
            break;
        case 2:
            if(mouse_packet_ready)
                break;
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