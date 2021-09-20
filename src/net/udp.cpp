#include <net/udp.h>

using namespace net;

//#UdpHandler::UdpHandler-doc: Empty constructor.
UdpHandler::UdpHandler() {

}

//#UdpHandler::~UdpHandler-doc: Empty constructor.
UdpHandler::~UdpHandler() {

}

//#UdpHandler::onUdpMessage-doc: Virtual function to be overridden. Called when the UDP client/server receives a message.
void UdpHandler::onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size) {

}

//#UdpSocket::UdpSocket-doc: UdpSocket constructor.
UdpSocket::UdpSocket(UdpProvider* provider) {
	this->provider = provider;
	this->handler = nullptr;
	this->listening = false;
}

//#UdpSocket::~UdpSocket-doc: Empty destructor.
UdpSocket::~UdpSocket() {

}

//#UdpSocket::handleUdpMessage-doc: Called when the UDP client/server receives a message.
void UdpSocket::handleUdpMessage(uint8_t* data, size_t size) {
	if (this->handler != nullptr) {
		this->handler->onUdpMessage(this, data, size);
	}
}

//#UdpSocket::send-doc: Send some data using a UDP socket.
void UdpSocket::send(uint8_t* data, size_t size) {
	provider->send(this, data, size);
}

//#UdpSocket::disconnect-doc: Disconnect a socket from the client/server.
void UdpSocket::disconnect() {
	provider->disconnect(this);
}

//#UdpProvider::UdpProvider-doc: Empty constructor.
UdpProvider::UdpProvider(Ipv4Provider *ipv4Provider): Ipv4Handler(ipv4Provider, 0x11), binds(100) {

}

struct udp_search_t {
	uint32_t src_ip;
	uint32_t dst_ip;
	udp_header_t* udp_header;
};

//#UdpProvider::onInternetProtocolReceived-doc: Called when IP receives a message.
bool UdpProvider::onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size) {
	if (size < sizeof(udp_header_t)) {
		return false;
	}

	udp_header_t* udp = (udp_header_t*)payload;

	udp_search_t search = {
		.src_ip = srcIP_BE,
		.dst_ip = dstIP_BE,
		.udp_header = udp
	};

	listv2<udp_bind_t>::node* n = binds.find<udp_search_t>([](udp_search_t u, listv2<udp_bind_t>::node* n) {
		if (n->data.handler->localPort == u.udp_header->dst_port && n->data.handler->localIp == u.dst_ip && n->data.handler->listening ) {
			n->data.handler->listening = false;
			n->data.handler->remotePort = u.udp_header->src_port;
			n->data.handler->remoteIp = u.src_ip;
			return true;
		}

		if (n->data.handler->remotePort == u.udp_header->src_port && (n->data.handler->remoteIp == u.src_ip || u.dst_ip == 0xFFFFFFFF || u.src_ip == 0xFFFFFFFF || n->data.handler->remoteIp == 0xFFFFFFFF)) {
			return true;
		}

		return false;
	}, search);

	if (n == nullptr) {
		return false;
	}

	UdpSocket* socket = n->data.handler;

	socket->handleUdpMessage(payload + sizeof(udp_header_t), size - sizeof(udp_header_t));

	return false;
}

//#UdpProvider::connect-doc: Connect to a UDP server using a given IP address and port.
UdpSocket* UdpProvider::connect(uint32_t ip, uint16_t port) {
	UdpSocket* socket = new UdpSocket(this);

	socket->remoteIp = ip;
	socket->remotePort = port;
	socket->localPort = free_port++;
	socket->localIp = backend->backend->nic->get_ip();

	socket->localPort = __builtin_bswap16(socket->localPort);
	socket->remotePort = __builtin_bswap16(socket->remotePort);

	udp_bind_t bind = {
		.port = socket->localPort,
		.handler = socket
	};

	this->binds.add(bind);

	return socket;
}

//#UdpProvider::listen-doc: Open a UDP server on a port.
UdpSocket* UdpProvider::listen(uint16_t port) {
	UdpSocket* socket = new UdpSocket(this);

	socket->localPort = port;
	socket->localIp = backend->backend->nic->get_ip();
	socket->listening = true;

	socket->localPort = __builtin_bswap16(socket->localPort);

	udp_bind_t bind = {
		.port = socket->localPort,
		.handler = socket
	};

	this->binds.add(bind);

	return socket;
}

//#UdpProvider::disconnect-doc: Disconnect a socket from a client/server.
void UdpProvider::disconnect(UdpSocket* socket) {
	listv2<udp_bind_t>::node* n = binds.find<UdpSocket*>([](UdpSocket* s, listv2<udp_bind_t>::node* n) {
		return s == n->data.handler;
	}, socket);

	n->data.handler->~UdpSocket();
	free(n->data.handler);

	binds.remove(n);
}

//#UdpProvider::send-doc: Send some data using a UDP socket.
void UdpProvider::send(UdpSocket* socket, uint8_t* data, size_t size) {
	uint16_t total_size = size + sizeof(udp_header_t);
	uint8_t* packet = (uint8_t*)malloc(total_size);

	udp_header_t* udp = (udp_header_t*)packet;

	udp->src_port = socket->localPort;
	udp->dst_port = socket->remotePort;
	udp->length = __builtin_bswap16(total_size);

	memcpy(packet + sizeof(udp_header_t), data, size);

	udp->checksum = 0;

	Ipv4Handler::send(socket->remoteIp, packet, total_size);

	free(packet);
}

//#UdpProvider::bind-doc: Bind a UDP handler to a socket.
void UdpProvider::bind(UdpSocket* socket, UdpHandler* handler) {
	socket->handler = handler;
}