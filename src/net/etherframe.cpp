#include <net/etherframe.h>
#include <driver/serial.h>
#include <paging/page_frame_allocator.h>
#include <string.h>

using namespace net;

//#EtherFrameHandler::EtherFrameHandler-doc: EtherFrameHandler constructor.
EtherFrameHandler::EtherFrameHandler(EtherFrameProvider* backend, uint16_t ether_type) {
	this->ether_type_be = ((ether_type & 0x00FF) << 8) | ((ether_type & 0xFF00) >> 8);
	this->backend = backend;
	backend->handlers.add(this);
}

//#EtherFrameHandler::~EtherFrameHandler-doc: EtherFrameHandler destructor.
EtherFrameHandler::~EtherFrameHandler() {
	this->backend->handlers.remove(this->backend->handlers.find<EtherFrameHandler*>([](EtherFrameHandler* h, listv2<EtherFrameHandler*>::node* n) {
		return h == n->data;
	}, this));
}

//#EtherFrameHandler::onEtherFrameReceived-doc: Virtual function to be overridden. Called when the ether frame receives a message.
bool EtherFrameHandler::onEtherFrameReceived(uint8_t* payload, uint32_t size) {
	driver::global_serial_driver->printf("Unhandlet etherframe package!\n");
	return false;
}

//#EtherFrameHandler::send-doc: Send some data with the ether frame.
void EtherFrameHandler::send(uint64_t dest_mac_be, uint8_t* payload, uint32_t size) {
	this->backend->send_f(dest_mac_be, this->ether_type_be, payload, size);
}

//#EtherFrameProvider::EtherFrameProvider-doc: Empty constructor.
EtherFrameProvider::EtherFrameProvider(int nic_id) : driver::nic::NicDataManager(nic_id), handlers(100) {

}

//#EtherFrameProvider::~EtherFrameProvider-doc: Empty destructor.
EtherFrameProvider::~EtherFrameProvider() {

}

//#EtherFrameProvider::recv-doc: Called when the network card gets a message.
bool EtherFrameProvider::recv(uint8_t* data, int32_t size) {
	ether_frame_header_t* frame = (ether_frame_header_t*) data;

	bool send_back = false;

	if (frame->dest_mac_be == 0xFFFFFFFFFFFF || frame->dest_mac_be == nic->get_mac()) {
		listv2<EtherFrameHandler*>::node* n = this->handlers.find<uint16_t>([](uint16_t t, listv2<EtherFrameHandler*>::node* n) {
			return t == n->data->ether_type_be;
		}, frame->ether_type_be);

		if (n != nullptr) {
			send_back = n->data->onEtherFrameReceived(data + sizeof(ether_frame_header_t), size - sizeof(ether_frame_header_t));
		} else {
			driver::global_serial_driver->printf("Unhandled etherframe!\n");
		}
	}

	if (send_back) {
		frame->dest_mac_be = frame->src_mac_be;
		frame->src_mac_be = this->nic->get_mac();
	}

	return send_back;
}

//#EtherFrameProvider::send_f-doc: Send a etherframe message.
void EtherFrameProvider::send_f(uint64_t dest_mac_be, uint16_t ether_type_be, uint8_t* payload, uint32_t size) {
	uint8_t* buffer = (uint8_t*) global_allocator.request_page();

	ether_frame_header_t* frame = (ether_frame_header_t*) buffer;

	frame->dest_mac_be = dest_mac_be;
	frame->src_mac_be = this->nic->get_mac();
	frame->ether_type_be = ether_type_be;

	memcpy(buffer + sizeof(ether_frame_header_t), payload, size);


	this->nic->send(buffer, size + sizeof(ether_frame_header_t));

	global_allocator.free_page(buffer);

}