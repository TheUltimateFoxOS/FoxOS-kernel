#include <driver/keyboard.h>

#include <renderer/font_renderer.h>
#include <driver/serial.h>

using namespace driver;

char keymap_de(uint8_t key, bool l_shift, bool r_shift, bool caps_lock){
	if(l_shift || r_shift || caps_lock){
		switch(key){
			case 0x02: return('!'); break;
			case 0x03: return('"'); break;
			//case 0x04: return('§'); break;
			case 0x05: return('$'); break;
			case 0x06: return('%'); break;
			case 0x07: return('&'); break;
			case 0x08: return('/'); break;
			case 0x09: return('('); break;
			case 0x0A: return(')'); break;
			case 0x0B: return('='); break;
			case 0x0C: return('?'); break;
			case 0x0D: return('`'); break;
			case 0x10: return('Q'); break;
			case 0x11: return('W'); break;
			case 0x12: return('E'); break;
			case 0x13: return('R'); break;
			case 0x14: return('T'); break;
			case 0x15: return('Z'); break;
			case 0x16: return('U'); break;
			case 0x17: return('I'); break;
			case 0x18: return('O'); break;
			case 0x19: return('P'); break;
			case 0x1E: return('A'); break;
			case 0x1F: return('S'); break;
			case 0x20: return('D'); break;
			case 0x21: return('F'); break;
			case 0x22: return('G'); break;
			case 0x23: return('H'); break;
			case 0x24: return('J'); break;
			case 0x25: return('K'); break;
			case 0x26: return('L'); break;
			case 0x2C: return('Y'); break;
			case 0x2D: return('X'); break;
			case 0x2E: return('C'); break;
			case 0x2F: return('V'); break;
			case 0x30: return('B'); break;
			case 0x31: return('N'); break;
			case 0x32: return('M'); break;
			case 0x33: return(';'); break;
			case 0x34: return(':'); break;
			case 0x35: return('_'); break;
			case 0x1b: return('*'); break;
			case 0x1C: return('\n'); break;
			case 0x2b: return('\''); break;
			//case 0x29: return('°'); break;
			case 0x39: return(' '); break;
			default:
				break;
		}
	}
	else{
		switch(key){
			case 0x02: return('1'); break;
			case 0x03: return('2'); break;
			case 0x04: return('3'); break;
			case 0x05: return('4'); break;
			case 0x06: return('5'); break;
			case 0x07: return('6'); break;
			case 0x08: return('7'); break;
			case 0x09: return('8'); break;
			case 0x0A: return('9'); break;
			case 0x0B: return('0'); break;
			//case 0x0C: return('ß'); break;
			//case 0x0D: return('´'); break;
			case 0x0E: return('\b'); break;
			case 0x10: return('q'); break;
			case 0x11: return('w'); break;
			case 0x12: return('e'); break;
			case 0x13: return('r'); break;
			case 0x14: return('t'); break;
			case 0x15: return('z'); break;
			case 0x16: return('u'); break;
			case 0x17: return('i'); break;
			case 0x18: return('o'); break;
			case 0x19: return('p'); break;
			case 0x1E: return('a'); break;
			case 0x1F: return('s'); break;
			case 0x20: return('d'); break;
			case 0x21: return('f'); break;
			case 0x22: return('g'); break;
			case 0x23: return('h'); break;
			case 0x24: return('j'); break;
			case 0x25: return('k'); break;
			case 0x26: return('l'); break;
			case 0x2C: return('y'); break;
			case 0x2D: return('x'); break;
			case 0x2E: return('c'); break;
			case 0x2F: return('v'); break;
			case 0x30: return('b'); break;
			case 0x31: return('n'); break;
			case 0x32: return('m'); break;
			case 0x33: return(','); break;
			case 0x34: return('.'); break;
			case 0x35: return('-'); break;
			case 0x1b: return('+'); break;
			case 0x1C: return('\n'); break;
			case 0x2b: return('#'); break;
			case 0x29: return('^'); break;
			case 0x39: return(' '); break;
			default:
				break;
		}
	}
	return 0;
}

char keymap_us(uint8_t key, bool l_shift, bool r_shift, bool caps_lock){
	if(l_shift || r_shift || caps_lock){
		switch(key){
			case 0x02: return('!'); break;
			case 0x03: return('\"'); break;
			case 0x04: break; //Pound symbol
			case 0x05: return('$'); break;
			case 0x06: return('%'); break;
			case 0x07: return(94); break;
			case 0x08: return('&'); break;
			case 0x09: return('*'); break;
			case 0x0A: return('('); break;
			case 0x0B: return(')'); break;
			case 0x0C: return('_'); break;
			case 0x0D: return('+'); break;
			case 0x0E: return('\b'); break;
			case 0x10: return('Q'); break;
			case 0x11: return('W'); break;
			case 0x12: return('E'); break;
			case 0x13: return('R'); break;
			case 0x14: return('T'); break;
			case 0x15: return('Y'); break;
			case 0x16: return('U'); break;
			case 0x17: return('I'); break;
			case 0x18: return('O'); break;
			case 0x19: return('P'); break;
			case 0x1A: return('{'); break;
			case 0x1B: return('}'); break;
			case 0x1E: return('A'); break;
			case 0x1F: return('S'); break;
			case 0x20: return('D'); break;
			case 0x21: return('F'); break;
			case 0x22: return('G'); break;
			case 0x23: return('H'); break;
			case 0x24: return('J'); break;
			case 0x25: return('K'); break;
			case 0x26: return('L'); break;
			case 0x27: return(':'); break;
			case 0x28: return('@'); break;
			case 0x29: return(126); break;
			case 0x2B: return(124); break;
			case 0x2C: return('Z'); break;
			case 0x2D: return('X'); break;
			case 0x2E: return('C'); break;
			case 0x2F: return('V'); break;
			case 0x30: return('B'); break;
			case 0x31: return('N'); break;
			case 0x32: return('M'); break;
			case 0x33: return('<'); break;
			case 0x34: return('>'); break;
			case 0x35: return('?'); break;
			case 0x1C: return('\n'); break;
			case 0x39: return(' '); break;
			default:
				break;
		}
	}
	else{
		switch(key){
			case 0x02: return('1'); break;
			case 0x03: return('2'); break;
			case 0x04: return('3'); break;
			case 0x05: return('4'); break;
			case 0x06: return('5'); break;
			case 0x07: return('6'); break;
			case 0x08: return('7'); break;
			case 0x09: return('8'); break;
			case 0x0A: return('9'); break;
			case 0x0B: return('0'); break;
			case 0x0C: return('-'); break;
			case 0x0D: return('='); break;
			case 0x0E: return('\b'); break;
			case 0x10: return('q'); break;
			case 0x11: return('w'); break;
			case 0x12: return('e'); break;
			case 0x13: return('r'); break;
			case 0x14: return('t'); break;
			case 0x15: return('y'); break;
			case 0x16: return('u'); break;
			case 0x17: return('i'); break;
			case 0x18: return('o'); break;
			case 0x19: return('p'); break;
			case 0x1A: return('['); break;
			case 0x1B: return(']'); break;
			case 0x1C: return('\n'); break;
			case 0x1E: return('a'); break;
			case 0x1F: return('s'); break;
			case 0x20: return('d'); break;
			case 0x21: return('f'); break;
			case 0x22: return('g'); break;
			case 0x23: return('h'); break;
			case 0x24: return('j'); break;
			case 0x25: return('k'); break;
			case 0x26: return('l'); break;
			case 0x27: return(';'); break;
			case 0x28: return('\''); break;
			case 0x29: return('#'); break;
			case 0x2B: return('\\'); break;
			case 0x2C: return('z'); break;
			case 0x2D: return('x'); break;
			case 0x2E: return('c'); break;
			case 0x2F: return('v'); break;
			case 0x30: return('b'); break;
			case 0x31: return('n'); break;
			case 0x32: return('m'); break;
			case 0x33: return(','); break;
			case 0x34: return('.'); break;
			case 0x35: return('/'); break;
			case 0x39: return(' '); break;
			default:
				break;
		}
	}	
	return 0;
}

char keymap_fr(uint8_t key, bool l_shift, bool r_shift, bool caps_lock){
	if(l_shift || r_shift || caps_lock){
		switch(key){
			case 0x02: return('1'); break;
			case 0x03: return('2'); break;
			case 0x04: return('3'); break;
			case 0x05: return('4'); break;
			case 0x06: return('5'); break;
			case 0x07: return('6'); break;
			case 0x08: return('7'); break;
			case 0x09: return('8'); break;
			case 0x0A: return('9'); break;
			case 0x0B: return('0'); break;
			case 0x0C: return(176); break;
			case 0x0D: return('_'); break;
			case 0x0E: return('\b'); break;
			case 0x10: return('A'); break;
			case 0x11: return('Z'); break;
			case 0x12: return('E'); break;
			case 0x13: return('R'); break;
			case 0x14: return('T'); break;
			case 0x15: return('Y'); break;
			case 0x16: return('U'); break;
			case 0x17: return('I'); break;
			case 0x18: return('O'); break;
			case 0x19: return('P'); break;
			case 0x1A: return(168); break;
			case 0x1B: return(163); break;
			case 0x1E: return('Q'); break;
			case 0x1F: return('S'); break;
			case 0x20: return('D'); break;
			case 0x21: return('F'); break;
			case 0x22: return('G'); break;
			case 0x23: return('H'); break;
			case 0x24: return('J'); break;
			case 0x25: return('K'); break;
			case 0x26: return('L'); break;
			case 0x27: return('M'); break;
			case 0x28: return('%'); break;
			case 0x2C: return('W'); break;
			case 0x2D: return('X'); break;
			case 0x2E: return('C'); break;
			case 0x2F: return('V'); break;
			case 0x30: return('B'); break;
			case 0x31: return('N'); break;
			case 0x32: return('?'); break;
			case 0x33: return('.'); break;
			case 0x34: return('/'); break;
			case 0x35: return(167); break;
			case 0x1C: return('\n'); break;
			case 0x39: return(' '); break;
			case 0x56: return('>'); break;
			default:
				break;
		}
	}
	else{
		switch(key){
			case 0x02: return('&'); break;
			case 0x03: return(233); break;
			case 0x04: return('"'); break;
			case 0x05: return('\''); break;
			case 0x06: return('('); break;
			case 0x07: return('-'); break;
			case 0x08: return(232); break;
			case 0x09: return('_'); break;
			case 0x0A: return(231); break;
			case 0x0B: return(224); break;
			case 0x0C: return(')'); break;
			case 0x0D: return('='); break;
			case 0x0E: return('\b'); break;
			case 0x10: return('a'); break;
			case 0x11: return('z'); break;
			case 0x12: return('e'); break;
			case 0x13: return('r'); break;
			case 0x14: return('t'); break;
			case 0x15: return('y'); break;
			case 0x16: return('u'); break;
			case 0x17: return('i'); break;
			case 0x18: return('o'); break;
			case 0x19: return('p'); break;
			case 0x1A: return('^'); break;
			case 0x1B: return('$'); break;
			case 0x1E: return('q'); break;
			case 0x1F: return('s'); break;
			case 0x20: return('d'); break;
			case 0x21: return('f'); break;
			case 0x22: return('g'); break;
			case 0x23: return('h'); break;
			case 0x24: return('j'); break;
			case 0x25: return('k'); break;
			case 0x26: return('l'); break;
			case 0x27: return('m'); break;
			case 0x28: return(249); break;
			case 0x29: return('*'); break;
			case 0x2C: return('w'); break;
			case 0x2D: return('x'); break;
			case 0x2E: return('c'); break;
			case 0x2F: return('v'); break;
			case 0x30: return('b'); break;
			case 0x31: return('n'); break;
			case 0x32: return(','); break;
			case 0x33: return(';'); break;
			case 0x34: return(':'); break;
			case 0x35: return('!'); break;
			case 0x1C: return('\n'); break;
			case 0x39: return(' '); break;
			case 0x56: return('<'); break;
			default:
				break;
		}
	}
	return 0;
}

KeyboardEventHandler::KeyboardEventHandler() {

}

void KeyboardEventHandler::KeyDown(char c) {

}

void KeyboardEventHandler::SpecialKeyDown(special_key key) {

}

void KeyboardEventHandler::SpecialKeyUp(special_key key) {

}

KeyboardDriver::KeyboardDriver(KeyboardEventHandler* handler) : interrupts::InterruptHandler(0x21), dataport(0x60), commandport(0x64) {
	this->handler = handler;
	this->keymap = keymap_us_e;
}

void KeyboardDriver::activate() {
	while(commandport.Read() & 0x1) {
		dataport.Read();
	}
	commandport.Write(0xae);
	commandport.Write(0x20);
	uint8_t status = (dataport.Read() | 1) & ~0x10;
	commandport.Write(0x60);
	dataport.Write(status);
	dataport.Write(0xf4);
}

bool KeyboardDriver::is_presend() {
	return true;
}

void KeyboardDriver::handle(){
	uint8_t key = dataport.Read();
	if (this->debug_mode) {
		driver::global_serial_driver->printf("KEY EVENT: %x\n", key);
	}
	if(handler == 0)
		return;

	if (this->extended_ascii) {
		this->extended_ascii = false;

		switch (key)
		{
			case 0x1D: //Right control down
				handler->SpecialKeyDown(special_key::right_control);
				break;
			case 0x9D: //Right control up
				handler->SpecialKeyUp(special_key::right_control);
				break;

			case 0x38: //Right alt down
				handler->SpecialKeyDown(special_key::right_alt);
				break;
			case 0xB8: //Right alt up
				handler->SpecialKeyUp(special_key::right_alt);
				break;

			case 0x5B: //Left GUI down (Windows/Apple)
				handler->SpecialKeyDown(special_key::left_gui);
				break;
			case 0xDB: //Left GUI up
				handler->SpecialKeyUp(special_key::left_gui);
				break;

			case 0x5C: //Right GUI down (Windows/Apple)
				handler->SpecialKeyDown(special_key::right_gui);
				break;
			case 0xDC: //Right GUI up
				handler->SpecialKeyUp(special_key::right_gui);
				break;
		
			case 0x48: //Up arrow down
				handler->SpecialKeyDown(special_key::up_arrow);
				break;
			case 0xC8: //Up arrow up
				handler->SpecialKeyUp(special_key::up_arrow);
				break;

			case 0x50: //Down arrow down
				handler->SpecialKeyDown(special_key::down_arrow);
				break;
			case 0xD0: //Down arrow up
				handler->SpecialKeyUp(special_key::down_arrow);
				break;

			case 0x4B: //Left arrow down
				handler->SpecialKeyDown(special_key::left_arrow);
				break;
			case 0xCB: //Left arrow up
				handler->SpecialKeyUp(special_key::left_arrow);
				break;

			case 0x4D: //Right arrow down
				handler->SpecialKeyDown(special_key::right_arrow);
				break;
			case 0xCD: //Right arrow up
				handler->SpecialKeyUp(special_key::right_arrow);
				break;

			default:
				//renderer::global_font_renderer->printf(" - 0xE0 0x%x", key);
				break;
		}
	} else {
		switch (key)
		{
			case 0xE0: //Extended ASCII
				this->extended_ascii = true;
				break;

			case 0x2A: //Left shift down
				this->l_shift = true;
				handler->SpecialKeyDown(special_key::left_shift);
				break;
			case 0xAA: //Left shift up
				this->l_shift = false;
				handler->SpecialKeyUp(special_key::left_shift);
				break;
			
			case 0x36: //Right shift down
				this->r_shift = true;
				handler->SpecialKeyDown(special_key::right_shift);
				break; //Right shift up
			case 0xB6:
				this->r_shift = false;
				handler->SpecialKeyUp(special_key::right_shift);
				break;

			case 0x1D: //Left control down
				handler->SpecialKeyDown(special_key::left_ctrl);
				break;
			case 0x9D: //Left control up
				handler->SpecialKeyUp(special_key::left_ctrl);
				break;

			case 0x38: //Left alt down
				handler->SpecialKeyDown(special_key::left_alt);
				break;
			case 0xB8: //Left alt up
				handler->SpecialKeyUp(special_key::left_alt);
				break;

			case 0x3A: //CapsLock down
				handler->SpecialKeyDown(special_key::caps_lock);
				this->caps_lock = !this->caps_lock;
				break;
			case 0xBA: //CapsLock up
				handler->SpecialKeyUp(special_key::caps_lock);
				break;

			case 0x45: //NumLock down
				handler->SpecialKeyDown(special_key::num_lock);
				break;
			case 0xC5: //NumLock up
				handler->SpecialKeyUp(special_key::num_lock);
				break;

			case 0x46: //ScrollLock down
				handler->SpecialKeyDown(special_key::scroll_lock);
				break;
			case 0xC6: //ScrollLock up
				handler->SpecialKeyUp(special_key::scroll_lock);
				break;
			
			default:
				//renderer::global_font_renderer->printf(" - 0x%x", key);
				//break;

				switch(this->keymap) {
					case keymap_de_e:
						handler->KeyDown(keymap_de(key, this->l_shift, this->r_shift, this->caps_lock));
						break;
					case keymap_fr_e:
						handler->KeyDown(keymap_fr(key, this->l_shift, this->r_shift, this->caps_lock));
						break;
					case keymap_us_e:
						handler->KeyDown(keymap_us(key, this->l_shift, this->r_shift, this->caps_lock));
						break;
					default:
						renderer::global_font_renderer->printf("Invalid keymap selected\n");
						break;
					}
				break;
		}
	}
}

char* KeyboardDriver::get_name() {
	return (char*) "keyboard";
}