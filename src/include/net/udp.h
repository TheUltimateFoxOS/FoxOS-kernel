#pragma once

#include <net/ipv4.h>
#include <net/listv2.h>

namespace net {

	struct udp_header_t {
		uint16_t src_port;
		uint16_t dst_port;
		uint16_t length;
		uint16_t checksum;
	} __attribute__((packed));

	class UdpSocket;
	class UdpProvider;


	class UdpHandler {
		public:
			UdpHandler();
			~UdpHandler();

			virtual void onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size);
	};

	class UdpSocket {
		public:
			UdpSocket(UdpProvider *provider);
			~UdpSocket();

			virtual void handleUdpMessage(uint8_t* data, size_t size);
			virtual void send(uint8_t* data, size_t size);
			virtual void disconnect();

			bool listening;
			uint16_t remotePort;
			uint32_t remoteIp;
			uint16_t localPort;
			uint32_t localIp;

			UdpProvider* provider;
			UdpHandler* handler;
	};

	class UdpProvider: public Ipv4Handler {
		public:

			struct udp_bind_t {
				uint16_t port;
				UdpSocket* handler;
			};

			list<udp_bind_t> binds;

			int free_port = 1024;
			

			UdpProvider(Ipv4Provider *ipv4Provider);
			~UdpProvider();

			virtual bool onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size);

			virtual UdpSocket* connect(uint32_t ip, uint16_t port);
			virtual UdpSocket* listen(uint16_t port);

			virtual void disconnect(UdpSocket* socket);

			virtual void send(UdpSocket* socket, uint8_t* data, size_t size);

			virtual void bind(UdpSocket* socket, UdpHandler* handler);
	};
}