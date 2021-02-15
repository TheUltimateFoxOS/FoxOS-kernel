#include <driver/driver.h>

using namespace driver;

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

void DriverManager::activate_all(bool force) {
	for(int i = 0; i < this->num_drivers; i++) {
		if(force) {
			this->drivers[i]->activate();
		} else {
			if(this->drivers[i]->is_presend()) {
				this->drivers[i]->activate();
			}
		}
	}
}