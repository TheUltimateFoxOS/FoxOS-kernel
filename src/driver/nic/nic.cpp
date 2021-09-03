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

void Nic::register_nic_data_manager(NicDataManager* nic_data_manager) {
	this->nic_data_manager = nic_data_manager;
	nic_data_manager->nic = this;
}

uint64_t Nic::get_mac() {
	return 0;
}

uint32_t Nic::get_ip() {
	return 0;
}

void Nic::set_ip(uint32_t ip) {
	
}

NicDataManager::NicDataManager(int id) {
	this->nic_id = id;
	global_nic_manager->get_nic(id)->register_nic_data_manager(this);
	driver::global_serial_driver->printf("NicDataManager created for nic id %d\n", id);
}

NicDataManager::~NicDataManager() {

}

void NicDataManager::send(uint8_t* data, int32_t size) {
	this->nic->send(data, size);
}

bool NicDataManager::recv(uint8_t* data, int32_t size) {
	driver::global_serial_driver->printf("Received unhandled package!\n");
	return false;
}


NicManager::NicManager() {
	this->num_Nics = 0;
}

void NicManager::add_Nic(Nic* Nic) {
	this->Nics[this->num_Nics] = Nic;
	driver::global_serial_driver->printf("Adding new Nic at idx %d!\n", this->num_Nics);
	this->num_Nics++;
}

Nic* NicManager::get_nic(int nic_id) {
	return this->Nics[nic_id];
}