#include <driver/nic/nic.h>

#include <renderer/font_renderer.h>
#include <driver/serial.h>

using namespace driver;
using namespace driver::nic;

NicManager* driver::nic::global_nic_manager;

Nic::Nic() {

}

Nic::~Nic() {

}

void Nic::send(uint8_t* data, int32_t size) {

}

void Nic::recv(nic_recv_handler handler) {
	this->handler = handler;
}



NicManager::NicManager() {
	this->num_Nics = 0;
}

void NicManager::add_Nic(Nic* Nic) {
	this->Nics[this->num_Nics] = Nic;
	driver::global_serial_driver->printf("Adding new Nic at idx %d!\n", this->num_Nics);
	this->num_Nics++;
}

void NicManager::send(int id, uint8_t* data, int32_t size) {
	if (id < this->num_Nics) {
		this->Nics[id]->send(data, size);
	}
}

void NicManager::recv(int id, nic_recv_handler handler) {
	if (id < this->num_Nics) {
		this->Nics[id]->recv(handler);
	}
}