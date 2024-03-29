#include <net/arp.h>
#include <driver/serial.h>

#include <scheduling/pit/pit.h>
#include <scheduling/hpet/hpet.h>

using namespace net;

//#AddressResolutionProtocol::AddressResolutionProtocol-doc: Constructor for the AddressResolutionProtocol class.
AddressResolutionProtocol::AddressResolutionProtocol(EtherFrameProvider* ether): EtherFrameHandler(ether, 0x806) {
	this->num_cache_entry = 0;
}

//#AddressResolutionProtocol::~AddressResolutionProtocol-doc: Empty destructor.
AddressResolutionProtocol::~AddressResolutionProtocol() {

}

//#AddressResolutionProtocol::onEtherFrameReceived-doc: Called when the ether frame receives some data.
bool AddressResolutionProtocol::onEtherFrameReceived(uint8_t* payload, uint32_t size) {
	if (size < sizeof(arp_message_t)) {
		return false;
	}

	arp_message_t* arp = (arp_message_t*) payload;
	if (arp->hardware_type == 0x0100) {
		if(arp->protocol == 0x0008 && arp->hardware_address_size == 6 && arp->protocol_address_size == 4 && arp->dest_ip == this->backend->nic->get_ip()) {
			switch (arp->command) {
				case 0x0100: // request
					{
						arp->command = 0x0200;
						arp->dest_ip = arp->src_ip;
						arp->dest_mac = arp->src_mac;
						arp->src_ip = this->backend->nic->get_ip();
						arp->src_mac = this->backend->nic->get_mac();
						this->send(arp->dest_mac, (uint8_t*) arp, sizeof(arp_message_t));
						return false;
					}
					break;
				case 0x0200: // response
					{
						if (this->num_cache_entry < 128) {
							this->ip_cache[this->num_cache_entry] = arp->src_ip;
							this->mac_cache[this->num_cache_entry] = arp->src_mac;
							this->num_cache_entry++;
						}
					}
					break;
			}
		}
	}

	return false;
}

//#AddressResolutionProtocol::broadcast_mac-doc: Broadcast the mac address from the current network card to the ip in the function arguments.
void AddressResolutionProtocol::broadcast_mac(uint32_t ip_be) {
	arp_message_t arp = {
		.hardware_type = 0x0100,
		.protocol = 0x0008,
		.hardware_address_size = 6,
		.protocol_address_size = 4,
		.command = 0x0200,
		.src_mac = this->backend->nic->get_mac(),
		.src_ip = this->backend->nic->get_ip(),
		.dest_mac = resolve(ip_be),
		.dest_ip = ip_be
	};

	this->send(arp.dest_mac, (uint8_t*) &arp, sizeof(arp_message_t));
}

//#AddressResolutionProtocol::request_mac_address-doc: Reaquest a mac address from the network card.
void AddressResolutionProtocol::request_mac_address(uint32_t ip_be) {
	arp_message_t arp = {
		.hardware_type = 0x0100,
		.protocol = 0x0008,
		.hardware_address_size = 6,
		.protocol_address_size = 4,
		.command = 0x0100,
		.src_mac = this->backend->nic->get_mac(),
		.src_ip = this->backend->nic->get_ip(),
		.dest_mac = 0xFFFFFFFFFFFF,
		.dest_ip = ip_be
	};

	this->send(arp.dest_mac, (uint8_t*) &arp, sizeof(arp_message_t));
}

//#AddressResolutionProtocol::get_mac_from_cache-doc: Read a mac address from the internal cache returns 0xFFFFFFFFFFFF if not found.
uint64_t AddressResolutionProtocol::get_mac_from_cache(uint32_t ip_be) {
	for (int i = 0; i < this->num_cache_entry; i++) {
		if (this->ip_cache[i] == ip_be) {
			return this->mac_cache[i];
		}
	} 
	return 0xFFFFFFFFFFFF;
}

//#AddressResolutionProtocol::resolve-doc: Resolve a mac address from a ip either using the cache or actualy sending the arp request.
uint64_t AddressResolutionProtocol::resolve(uint32_t ip_be) {
	if (ip_be == 0xffffffff) {
		return 0xFFFFFFFFFFFF;
	}
	
	uint64_t result = this->get_mac_from_cache(ip_be);

	if (result == 0xFFFFFFFFFFFF) {
		this->request_mac_address(ip_be);
	}

	int timeout = 100000;

	while (result == 0xFFFFFFFFFFFF) {
		result = this->get_mac_from_cache(ip_be);
		if (--timeout == 0) {
			driver::global_serial_driver->printf("timeout for arp request!\n");
			return 0;
		}

		if (hpet::is_available()) {
			hpet::sleep(10);
		} else {
			PIT::sleep(10);
		}
	}

	return result;
}