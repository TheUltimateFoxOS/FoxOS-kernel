#include <driver/nic/nic.h>

#include <renderer/font_renderer.h>
#include <driver/serial.h>

#include <net/net_stack.h>

using namespace driver;
using namespace driver::nic;

NicManager* driver::nic::global_nic_manager;

//#Nic::Nic-doc: Empty constructor.
Nic::Nic() {

}

//#Nic::~Nic-doc: Empty destructor.
Nic::~Nic() {

}

//#Nic::send-doc: Virtual function to be overridden. Send data with the nic.
void Nic::send(uint8_t* data, int32_t size) {

}

//#Nic::register_nic_data_manager-doc: Virtual function to be overridden. Register a nic data manager.
void Nic::register_nic_data_manager(NicDataManager* nic_data_manager) {
	this->nic_data_manager = nic_data_manager;
	nic_data_manager->nic = this;
}

//#Nic::get_mac-doc: Virtual function to be overridden. Get the nic's MAC address.
uint64_t Nic::get_mac() {
	return 0;
}

//#Nic::get_ip-doc: Virtual function to be overridden. Get the nic's IP address.
uint32_t Nic::get_ip() {
	return 0;
}

//#Nic::set_ip-doc: Virtual function to be overridden. Set the nic's IP address.
void Nic::set_ip(uint32_t ip) {
	
}

void Nic::load_network_stack(net::net_stack_t* network_stack) {
	this->network_stack = network_stack;

	driver::global_serial_driver->printf("[NIC] Network stack loaded.\n");
	driver::global_serial_driver->printf("      etherframe: %x\n", network_stack->ether);
	driver::global_serial_driver->printf("      arp: %x\n", network_stack->arp);
	driver::global_serial_driver->printf("      ipv4: %x\n", network_stack->ipv4);
	driver::global_serial_driver->printf("      icmp: %x\n", network_stack->icmp);
	driver::global_serial_driver->printf("      udp: %x\n", network_stack->udp);
	driver::global_serial_driver->printf("      tcp: %x\n", network_stack->tcp);
	driver::global_serial_driver->printf("      dns: %x\n", network_stack->dns);
}


//#NicDataManager::NicDataManager-doc: NicDataManager constructor.
NicDataManager::NicDataManager(int id) {
	this->nic_id = id;
	global_nic_manager->get_nic(id)->register_nic_data_manager(this);
	driver::global_serial_driver->printf("NicDataManager created for nic id %d\n", id);
}

//#NicDataManager::~NicDataManager-doc: NicDataManager destructor.
NicDataManager::~NicDataManager() {

}

//#NicDataManager::send-doc: Send data with the nic.
void NicDataManager::send(uint8_t* data, int32_t size) {
	this->nic->send(data, size);
}

//#NicDataManager::recv-doc: Virtual function to be overridden. This is called when data is recieved.
bool NicDataManager::recv(uint8_t* data, int32_t size) {
	driver::global_serial_driver->printf("Received unhandled package!\n");
	return false;
}

//#NicManager::NicManager-doc: NicManager constructor.
NicManager::NicManager() {
	this->num_Nics = 0;
}

//#NicManager::add_Nic-doc: Add a nic to the nic manager.
void NicManager::add_Nic(Nic* Nic) {
	this->Nics[this->num_Nics] = Nic;
	driver::global_serial_driver->printf("Adding new Nic at idx %d!\n", this->num_Nics);
	this->num_Nics++;
}

//#NicManager::get_nic-doc: Get a nic by ID.:
Nic* NicManager::get_nic(int nic_id) {
	return this->Nics[nic_id];
}