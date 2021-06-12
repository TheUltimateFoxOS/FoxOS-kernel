#include <driver/driver.h>

#include <renderer/font_renderer.h>
#include <string.h>

using namespace driver;

DriverManager* driver::global_driver_manager;

Driver::Driver() {

}

Driver::~Driver() {

}

void Driver::activate() {

}

bool Driver::is_presend() {
	return true;
}

char* Driver::get_name() {
	return (char*) "default";
}

DriverManager::DriverManager() {
	this->num_drivers = 0;
}

void DriverManager::add_driver(Driver* driver) {
	this->drivers[this->num_drivers] = driver;
	this->num_drivers++;
}

void DriverManager::set_status(char* status, uint64_t color) {
	renderer::global_font_renderer->cursor_position.x = renderer::global_font_renderer->target_frame_buffer->width - 8 * (strlen(status) + 4);
	renderer::global_font_renderer->printf("[%f%s%r]\n", color, status);
}


void DriverManager::activate_driver(bool force, Driver* driver) {
	if(force) {
		driver->activate();
		this->set_status((char*) "force", 0xffaa00ff);
	} else {
		if(driver->is_presend()) {
			driver->activate();
			this->set_status((char*) "ok", 0xff00ff00);
		} else {
			this->set_status((char*) "no device", 0xff787878);
		}
	}
}

void DriverManager::activate_all(bool force) {
	for(int i = 0; i < this->num_drivers; i++) {
		renderer::global_font_renderer->printf("Loading driver for device: %s", this->drivers[i]->get_name());
		activate_driver(force, this->drivers[i]);
	}
}