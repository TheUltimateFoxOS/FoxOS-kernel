#include <net/dhcp.h>
#include <config.h>

#include <scheduling/pit/pit.h>
#include <scheduling/hpet/hpet.h>

using namespace net;

//#DhcpProtocol::DhcpProtocol-doc: Constructor for the DHCP protocol. Takes a already opened and connected socket to a ipv4 broadcast.
DhcpProtocol::DhcpProtocol(UdpSocket* socket) {
	this->socket = socket;
	socket->localPort = __builtin_bswap16(68);
	this->ip = 0;
}

//#DhcpProtocol::~DhcpProtocol-doc: Empty destructor for the DHCP protocol.
DhcpProtocol::~DhcpProtocol() {

}

//#DhcpProtocol::request-doc: This function is used to send a dhcp discover or request message to the broadcast address.
void DhcpProtocol::request() {
	dhcp_packet_t packet;
	memset(&packet, 0, sizeof(dhcp_packet_t));

	make_dhcp_packet(&packet, 1, 0x00000000);
	socket->send((uint8_t*) &packet, sizeof(dhcp_packet_t));

	int timeout = 100000;

	while (!complete) {
		if (--timeout == 0) {
			break;
		}

		if (hpet::is_available()) {
			hpet::sleep(10);
		} else {
			PIT::sleep(10);
		}
	}
}

void DhcpProtocol::request(uint32_t ip) {
	dhcp_packet_t packet;
	memset(&packet, 0, sizeof(dhcp_packet_t));

	make_dhcp_packet(&packet, 3, ip);
	socket->send((uint8_t*) &packet, sizeof(dhcp_packet_t));
}

//#DhcpProtocol::onUdpMessage-doc: This function is called when a udp packet is received for the udp socket.
void DhcpProtocol::onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size) {
	dhcp_packet_t* packet = (dhcp_packet_t*) data;

	uint8_t* type = (uint8_t*) get_dhcp_options(packet, 53);

	switch (*type) {
		case 2:
			this->request(packet->your_ip);
			break;
		case 5:
			this->ip = packet->your_ip;
			this->gateway = *(uint32_t*) get_dhcp_options(packet, 3);
			this->dns = *(uint32_t*) get_dhcp_options(packet, 6);
			this->complete = true;
			uint32_t* subnet = (uint32_t*) get_dhcp_options(packet, 1);
			this->subnet = *subnet;
			break;
	}
}

//#DhcpProtocol::get_dhcp_options-doc: This function is used to get the dhcp options from a dhcp packet.
void* DhcpProtocol::get_dhcp_options(dhcp_packet_t* packet, uint8_t type) {
	uint8_t* options = packet->options + 4;
	uint8_t curr_type = *options;
	while(curr_type != 0xff) {
		uint8_t len = *(options + 1);
		if(curr_type == type) {
			return options + 2;
		}
		options += (2 + len);
		curr_type = *options;
	}

	return nullptr;
}

//#DhcpProtocol::make_dhcp_packet-doc: This function is used to make a dhcp packet.
void DhcpProtocol::make_dhcp_packet(dhcp_packet_t* packet, uint8_t msg_type, uint32_t request_ip) {
	packet->op = DHCP_REQUEST;
	packet->hardware_type = 1;
	packet->hardware_addr_len = 6;
	packet->hops = 0;
	packet->xid = __builtin_bswap32(DHCP_TRANSACTION_IDENTIFIER);
	packet->flags = __builtin_bswap16(0);
	uint64_t mac = socket->provider->backend->backend->nic->get_mac();
	memcpy(packet->client_hardware_addr, &mac, sizeof(mac));

	// Send dhcp packet using UDP
	uint8_t dst_ip[4];
	memset(dst_ip, 0xff, 4);

	// Options specific to DHCP Discover (required)

	// Magic Cookie
	uint8_t * options = packet->options;
	*((uint32_t*)(options)) = __builtin_bswap32(0x63825363);
	options += 4;

	// First option, message type = DHCP_DISCOVER/DHCP_REQUEST
	*(options++) = 53;
	*(options++) = 1;
	*(options++) = msg_type;

	// Client identifier
	*(options++) = 61;
	*(options++) = 0x07;
	*(options++) = 0x01;
	memcpy(options, &mac, sizeof(mac));
	options += 6;

	// Requested IP address
	*(options++) = 50;
	*(options++) = 0x04;
	*((uint32_t*)(options)) = __builtin_bswap32(0x0a00020e);
	memcpy((uint32_t*)(options), &request_ip, 4);
	options += 4;

	// Host Name
	*(options++) = 12;
	*(options++) = 1 + strlen(HOSTNAME);
	memcpy(options, HOSTNAME, strlen(HOSTNAME));
	options += strlen(HOSTNAME);
	*(options++) = 0x00;

	// Parameter request list
	
	*(options++) = 55;
	*(options++) = 8;
	*(options++) = 0x1; // Subnet mask
	*(options++) = 0x3; // Router
	*(options++) = 0x6; // Domain name server
	*(options++) = 0xf; // Domain name
	*(options++) = 0x2c; // NeTBIOS over TCP/IP name server
	*(options++) = 0x2e; // NeTBIOS over TCP/IP node type
	*(options++) = 0x2f; // NeTBIOS over TCP/IP scope
	*(options++) = 0x39; // Maximum DHCP message size

	*(options++) = 0xff; // End of dhcp packet
	

}