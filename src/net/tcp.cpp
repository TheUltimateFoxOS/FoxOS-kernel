#include <net/tcp.h>
#include <driver/serial.h>
#include <scheduling/pit/pit.h>

using namespace net;

//#TcpHandler::TcpHandler-doc: Empty constructor.
TcpHandler::TcpHandler() {

}

//#TcpHandler::~TcpHandler-doc: Empty destructor.
TcpHandler::~TcpHandler() {

}

//#TcpHandler::onTcpMessage-doc: Virtual function to be overridden. Called when the TCP client/server receives a message.
bool TcpHandler::onTcpMessage(TcpSocket* socket, uint8_t* data, size_t size) {
	return true;
}

//#TcpSocket::TcpSocket-doc: TcpSocket destrcutor.
TcpSocket::TcpSocket(TcpProvider* provider) {
	this->provider = provider;
	this->handler = nullptr;
	this->state = TcpSocketState::CLOSED;
}

//#TcpSocket::~TcpSocket-doc: Empty destrcutor.
TcpSocket::~TcpSocket() {

}

//#TcpSocket::handleTcpMessage-doc: Called when the TCP client/server receives a message.
bool TcpSocket::handleTcpMessage(uint8_t* data, size_t size) {
	if (this->handler != nullptr) {
		return this->handler->onTcpMessage(this, data, size);
	} else {
		return false;
	}
}

//#TcpSocket::send-doc: Send some data using a TCP socket.
void TcpSocket::send(uint8_t* data, size_t size) {
	uint64_t time = PIT::time_since_boot;
	while (this->state != TcpSocketState::ESTABLISHED) {
		if (PIT::time_since_boot > time + 1000) {
			driver::global_serial_driver->printf("TCP: Send timeout, connection not established.\n");
			return;
		}
	}
	provider->send(this, data, size, TcpFlag::PSH | TcpFlag::ACK);
}

//#TcpSocket::disconnect-doc: Disconnect a socket from the client/server.
void TcpSocket::disconnect() {
	provider->disconnect(this);
}

//#TcpProvider::TcpProvider-doc: Empty constructor.
TcpProvider::TcpProvider(Ipv4Provider *ipv4Provider): Ipv4Handler(ipv4Provider, 0x06), binds(100) {
}

struct tcp_search_t {
	uint32_t src_ip;
	uint32_t dst_ip;
	tcp_header_t* tcp_header;
};

//#TcpProvider::onInternetProtocolReceived-doc: Called when IP receives a message.
bool TcpProvider::onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size) {
	if (size < 20) {
		return false;
	}

	tcp_header_t* tcp = (tcp_header_t*)payload;

	tcp_search_t search = {
		.src_ip = srcIP_BE,
		.dst_ip = dstIP_BE,
		.tcp_header = tcp
	};

	listv2<tcp_bind_t>::node* n = binds.find<tcp_search_t>([](tcp_search_t u, listv2<tcp_bind_t>::node* n) {
		if (n->data.handler->localPort == u.tcp_header->dst_port && n->data.handler->localIp == u.dst_ip && n->data.handler->state == TcpSocketState::LISTEN && (u.tcp_header->flags & (TcpFlag::SYN | TcpFlag::ACK)) == TcpFlag::SYN) {
			return true;
		}

		if (n->data.handler->remotePort == u.tcp_header->src_port && (n->data.handler->remoteIp == u.src_ip || u.dst_ip == 0xFFFFFFFF || u.src_ip == 0xFFFFFFFF || n->data.handler->remoteIp == 0xFFFFFFFF)) {
			return true;
		}

		return false;
	}, search);

	if (n == nullptr) {
		return false;
	}

	TcpSocket* socket = n->data.handler;
	bool reset = false;

	if (socket != 0 && tcp->flags & TcpFlag::RST) {
		socket->state = TcpSocketState::CLOSED;
	}

	if (socket != 0 && socket->state != TcpSocketState::CLOSED) {
		switch((tcp->flags) & (TcpFlag::SYN | TcpFlag::ACK | TcpFlag::FIN)) {
			case TcpFlag::SYN:
				if (socket->state == TcpSocketState::LISTEN) {
					socket->state == TcpSocketState::SYN_RECEIVED;
					socket->remotePort = tcp->src_port;
					socket->remoteIp = srcIP_BE;
					socket->ack_num = __builtin_bswap32(tcp->seq_num) + 1;

					#warning The TCP sequence starting offset should be random
					socket->seq_num = 0xbeefcafe;

					send(socket, 0, 0, TcpFlag::SYN | TcpFlag::ACK);
					socket->seq_num++;
				} else {
					reset = true;
				}
				break;
			case TcpFlag::SYN | TcpFlag::ACK:
				if (socket->state == TcpSocketState::SYN_SENT) {
					socket->state = TcpSocketState::ESTABLISHED;
					socket->ack_num = __builtin_bswap32(tcp->seq_num) + 1;
					socket->seq_num++;
					driver::global_serial_driver->printf("TCP: SYN ACK.\n");
					send(socket, 0, 0, TcpFlag::ACK);
				} else {
					reset = true;
				}
				break;
			case TcpFlag::SYN | TcpFlag::FIN:
			case TcpFlag::SYN | TcpFlag::FIN | TcpFlag::ACK:
				reset = true;
				break;
			case TcpFlag::FIN:
			case TcpFlag::FIN | TcpFlag::ACK:
				if (socket->state == TcpSocketState::ESTABLISHED) {
					socket->state = TcpSocketState::CLOSE_WAIT;
					socket->ack_num++;
					send(socket, 0, 0, TcpFlag::ACK);
					send(socket, 0, 0, TcpFlag::FIN | TcpFlag::ACK);
					driver::global_serial_driver->printf("TCP: Socket closed.\n");
				} else if (socket->state == TcpSocketState::CLOSE_WAIT) {
					socket->state = TcpSocketState::CLOSED;
				} else if (socket->state == TcpSocketState::FIN_WAIT_1 || socket->state == TcpSocketState::FIN_WAIT_2) {
					socket->state = TcpSocketState::CLOSED;
					socket->ack_num++;
					driver::global_serial_driver->printf("TCP: Socket closed.\n");
					send(socket, 0, 0, TcpFlag::ACK);
				} else {
					reset = true;
				}
				break;
			case TcpFlag::ACK:
				if (socket->state == TcpSocketState::SYN_RECEIVED) {
					socket->state = TcpSocketState::ESTABLISHED;
					return false;
				} else if (socket->state == TcpSocketState::FIN_WAIT_1) {
					socket->state = TcpSocketState::FIN_WAIT_2;
					return false;
				} else if (socket->state == TcpSocketState::CLOSE_WAIT) {
					socket->state == TcpSocketState::CLOSED;
					break;
				} else if (*(payload + (tcp->data_offset * 4)) == 0) {
					break; //This is a bit scummy
				}
			default:
				if (__builtin_bswap32(tcp->seq_num) == socket->ack_num) {
					driver::global_serial_driver->printf("TCP: Processigng packet.\n");
					reset = !(socket->handleTcpMessage(payload + (tcp->data_offset * 4), size - (tcp->data_offset * 4)));
					if (!reset) {
						socket->ack_num += size - (tcp->data_offset * 4);
						send(socket, 0, 0, TcpFlag::ACK);
					}
				} else { //Data is in the wrong order
					driver::global_serial_driver->printf("TCP: Reset because packets are in the wrong order.\n");
					reset = true;
				}
		}
	}

	if (reset) {
		driver::global_serial_driver->printf("TCP: Conection was reset.\n");
		if (socket != 0) {
			send(socket, 0, 0, TcpFlag::RST);
		} else {
			TcpSocket socketTMP = TcpSocket(this);
			socketTMP.remotePort = tcp->src_port;
			socketTMP.remoteIp = srcIP_BE;
			socketTMP.localPort = tcp->dst_port;
			socketTMP.localIp = dstIP_BE;
			socketTMP.seq_num = __builtin_bswap32(tcp->ack);
			socketTMP.ack_num = __builtin_bswap32(tcp->seq_num) + 1;
			send(&socketTMP, 0, 0, TcpFlag::RST);
		}
		return false;
	}

	if (socket->state == TcpSocketState::CLOSED) {
		listv2<tcp_bind_t>::node* n = binds.find<TcpSocket*>([](TcpSocket* s, listv2<tcp_bind_t>::node* n) {
			return s == n->data.handler;
		}, socket);

		n->data.handler->~TcpSocket();
		free(n->data.handler);

		binds.remove(n);
	}

	return false;
}

//#TcpProvider::connect-doc: Connect to a TCP server using a given IP address and port.
TcpSocket* TcpProvider::connect(uint32_t ip, uint16_t port) {
	TcpSocket* socket = new TcpSocket(this);

	socket->remoteIp = ip;
	socket->remotePort = port;
	socket->localPort = free_port++;
	socket->localIp = backend->backend->nic->get_ip();

	socket->localPort = __builtin_bswap16(socket->localPort);
	socket->remotePort = __builtin_bswap16(socket->remotePort);

	tcp_bind_t bind = {
		.port = socket->localPort,
		.handler = socket
	};

	this->binds.add(bind);
	socket->state = TcpSocketState::SYN_SENT;

	#warning The TCP sequence starting offset should be random
	socket->seq_num = 0xbeefcafe;

	send(socket, 0, 0, TcpFlag::SYN);

	return socket;
}

//#TcpProvider::listen-doc: Open a TCP server on a port.
TcpSocket* TcpProvider::listen(uint16_t port) {
	TcpSocket* socket = new TcpSocket(this);

	socket->localPort = port;
	socket->localIp = backend->backend->nic->get_ip();
	socket->state = TcpSocketState::LISTEN;

	socket->localPort = __builtin_bswap16(socket->localPort);

	tcp_bind_t bind = {
		.port = socket->localPort,
		.handler = socket
	};

	this->binds.add(bind);

	return socket;
}

//#TcpProvider::disconnect-doc: Disconnect a socket from a client/server.
void TcpProvider::disconnect(TcpSocket* socket) {
	socket->state = TcpSocketState::FIN_WAIT_1;
	send(socket, 0, 0, TcpFlag::FIN + TcpFlag::ACK);
	socket->seq_num++;
}

//#TcpProvider::send-doc: Send some data using a TCP socket.
void TcpProvider::send(TcpSocket* socket, uint8_t* data, size_t size, uint16_t flags) {
	uint16_t total_size = size + sizeof(tcp_header_t);
	uint16_t total_size_phdr = total_size + sizeof(tcp_pseudo_header_t);

	uint8_t* packet = (uint8_t*)malloc(total_size_phdr);
	memset(packet, 0, total_size_phdr);

	tcp_pseudo_header_t* phdr = (tcp_pseudo_header_t*)packet;
	tcp_header_t* tcp = (tcp_header_t*)(packet + sizeof(tcp_pseudo_header_t));

	tcp->data_offset = sizeof(tcp_header_t) / 4;
	tcp->src_port = socket->localPort;
	tcp->dst_port = socket->remotePort;

	tcp->ack = __builtin_bswap32(socket->ack_num);
	tcp->seq_num = __builtin_bswap32(socket->seq_num);
	tcp->reserved = 0;
	tcp->flags = flags;
	#warning Maybe this should be set to actually look at the amount of RAM available
	tcp->window_size = 0xFFFF;
	tcp->urgent_ptr = 0;
	tcp->options = ((flags & TcpFlag::SYN) != 0 ? 0xB4050402 : 0);

	socket->seq_num += size;

	memcpy(packet + sizeof(tcp_header_t) + sizeof(tcp_pseudo_header_t), data, size);

	phdr->src_ip = socket->localIp;
	phdr->dst_ip = socket->remoteIp;
	phdr->protocol = 0x0600;
	phdr->total_len = __builtin_bswap16(total_size);

	tcp->checksum = 0;
	tcp->checksum = this->backend->checksum((uint16_t*) packet, total_size_phdr);

	Ipv4Handler::send(socket->remoteIp, (uint8_t*) tcp, total_size);

	free(packet);
}

//#TcpProvider::bind-doc: Bind a TCP handler to a socket.
void TcpProvider::bind(TcpSocket* socket, TcpHandler* handler) {
	socket->handler = handler;
}