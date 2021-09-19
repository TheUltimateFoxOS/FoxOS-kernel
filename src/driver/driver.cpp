#include <driver/driver.h>

#include <renderer/font_renderer.h>
#include <string.h>

using namespace driver;

DriverManager* driver::global_driver_manager;

//#Driver::Driver-doc: Empty constructor.
Driver::Driver() {

}

//#Driver::~Driver-doc: Empty destructor.
Driver::~Driver() {

}

//#Driver::activate-doc: Virtual function to be overridden. Activate the driver.
void Driver::activate() {

}

//#Driver::is_presend-doc: Virtual function to be overridden. Get if is pre send.
bool Driver::is_presend() {
	return true;
}

//#Driver::get_name-doc: Virtual function to be overridden. Get the driver's name.
char* Driver::get_name() {
	return (char*) "default";
}

//#DriverManager::DriverManager-doc: DriverManager constructor.
DriverManager::DriverManager() {
	this->num_drivers = 0;
}

//#DriverManager::add_driver-doc: Add a driver to the driver manager.
void DriverManager::add_driver(Driver* driver) {
	this->drivers[this->num_drivers] = driver;
	this->num_drivers++;
}

//#DriverManager::set_status-doc: Print the device status onto the screen.
void DriverManager::set_status(char* status, uint64_t color) {
	renderer::global_font_renderer->cursor_position.x = renderer::global_font_renderer->target_frame_buffer->width - 8 * (strlen(status) + 4);
	renderer::global_font_renderer->printf("[%f%s%r]\n", color, status);
}

//#DriverManager::activate_driver-doc: Activate a driver.
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

//#DriverManager::activate_all-doc: Activate all drivers.
void DriverManager::activate_all(bool force) {
	for(int i = 0; i < this->num_drivers; i++) {
		renderer::global_font_renderer->printf("Loading driver for device: %s", this->drivers[i]->get_name());
		activate_driver(force, this->drivers[i]);
	}
}

//#DriverManager::find_driver_by_name-doc: Find a driver by name.
Driver* DriverManager::find_driver_by_name(char* name) {
	for (int i = 0; i < num_drivers; i++) {
		if(strcmp(drivers[i]->get_name(), name) == 0) {
			return drivers[i];
		}
	}
	return nullptr;
}