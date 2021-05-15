#include <driver/driver.h>
#include <renderer/font_renderer.h>

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

DriverManager::DriverManager() {
	this->num_drivers = 0;
}

void DriverManager::add_driver(Driver* driver) {
	this->drivers[this->num_drivers] = driver;
	this->num_drivers++;
}

void DriverManager::activate_driver(bool force, Driver* driver) {
	if(force) {
		driver->activate();
	} else {
		if(driver->is_presend()) {
			driver->activate();
		}
	}
}

void DriverManager::activate_all(bool force) {
	for(int i = 0; i < this->num_drivers; i++) {
		activate_driver(force, this->drivers[i]);
	}
}