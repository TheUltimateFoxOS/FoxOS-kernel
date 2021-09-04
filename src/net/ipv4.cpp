#include <net/ipv4.h>
#include <driver/serial.h>

using namespace net;

Ipv4Handler::Ipv4Handler(Ipv4Provider* backend, uint8_t protocol) {
	this->protocol = protocol;
	this->backend = backend;
	backend->handlers.add(this);
}

Ipv4Handler::~Ipv4Handler() {
	this->backend->handlers.remove(this->backend->handlers.find<Ipv4Handler*>([](Ipv4Handler* h, list<Ipv4Handler*>::node* n) {
		return h == n->data;
	}, this));	
}

void Ipv4Handler::send(uint32_t dest_ip_be, uint8_t* payload, uint32_t size) {
	backend->send(dest_ip_be, this->protocol, payload, size);
}

bool Ipv4Handler::onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size) {
	return false;
}

Ipv4Provider::Ipv4Provider(EtherFrameProvider* backend, AddressResolutionProtocol* arp, uint32_t gateway_ip_be, uint32_t subnet_mask_be) : EtherFrameHandler(backend, 0x0800), handlers(100) {
	this->arp = arp;
	this->gateway_ip_be = gateway_ip_be;
	this->subnet_mask_be =subnet_mask_be;
}

Ipv4Provider::~Ipv4Provider() {

}

bool Ipv4Provider::onEtherFrameReceived(uint8_t* payload, uint32_t size) {
	if (size < sizeof(ipv4_message_t)) {
		return false;
	}

	ipv4_message_t* ipv4 = (ipv4_message_t*) payload;
	bool send_back = false;

	if (ipv4->destination_address == backend->nic->get_ip()) {
		int length = ipv4->total_length;
		if (length > size) {
			length = size;
		}

		list<Ipv4Handler*>::node* n = this->handlers.find<uint8_t>([](uint8_t t, list<Ipv4Handler*>::node* n) {
			return t == n->data->protocol;
		}, ipv4->protocol);

		if (n != nullptr) {
			send_back = n->data->onInternetProtocolReceived(ipv4->source_address, ipv4->destination_address, payload + 4 * ipv4->header_length, length - 4 * ipv4->header_length);
		} else {
			driver::global_serial_driver->printf("IPv4: Unknown protocol %d\n", ipv4->protocol);
		}
	}

	if (send_back) {
		uint32_t temp = ipv4->destination_address;
		ipv4->destination_address = ipv4->source_address;
		ipv4->source_address = temp;

		ipv4->time_to_live = 0x40;
		ipv4->header_checksum = checksum((uint8_t*) ipv4, 4 * ipv4->header_length);
		backend->send(payload, size);
	}

	return send_back;
}

void Ipv4Provider::send(uint32_t dest_ip_be, uint8_t protocol, uint8_t* payload, uint32_t size) {
	uint8_t* buffer = new uint8_t[size + sizeof(ipv4_message_t)];
	ipv4_message_t* ipv4 = (ipv4_message_t*) buffer;

	ipv4->version = 4;
	ipv4->header_length = sizeof(ipv4_message_t) / 4;
	ipv4->type_of_service = 0;
	ipv4->total_length = size + sizeof(ipv4_message_t);
	ipv4->total_length = ((ipv4->total_length & 0xFF00) >> 8) | ((ipv4->total_length & 0x00FF) << 8);
	ipv4->identification = 0x0100;
	ipv4->flags_and_fragment_offset = 0x0040;
	ipv4->time_to_live = 0x40;
	ipv4->protocol = protocol;
	ipv4->destination_address = dest_ip_be;
	ipv4->source_address = backend->nic->get_ip();

	ipv4->header_checksum = 0;
	ipv4->header_checksum = checksum(buffer, sizeof(ipv4_message_t));

	memcpy(buffer + sizeof(ipv4_message_t), payload, size);

	uint32_t route = dest_ip_be;
	if((dest_ip_be & subnet_mask_be) != (ipv4->source_address & subnet_mask_be)) {
		route = gateway_ip_be;
	}

	backend->send_f(arp->resolve(route), this->ether_type_be, buffer, size + sizeof(ipv4_message_t));

	delete[] buffer;
}

uint16_t Ipv4Provider::checksum(uint8_t* data, uint32_t size) {
	uint32_t temp = 0;
	
	for(int i = 0; i < size / 2; i++) {
		temp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);
	}

	if(size % 2) {
		temp += ((uint16_t)((char*)data)[size-1]) << 8;
	}

	while(temp & 0xFFFF0000) {
		temp = (temp & 0xFFFF) + (temp >> 16);
	}

	return ((temp & 0xFF00) >> 8) | ((temp & 0x00FF) << 8);
}