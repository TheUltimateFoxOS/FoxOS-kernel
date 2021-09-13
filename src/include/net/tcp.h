#pragma once

#include <net/ipv4.h>
#include <net/listv2.h>
#include <stdint.h>

namespace net {
	enum TcpSocketState {
		CLOSED = 0,
		LISTEN,
		SYN_SENT,
		SYN_RECEIVED,

		ESTABLISHED,

		FIN_WAIT_1,
		FIN_WAIT_2,
		CLOSING,
		TIME_WAIT,

		CLOSE_WAIT
	};

	enum TcpFlag {
		FIN = 1,
		SYN = 2,
		RST = 4,
		PSH = 8,
		ACK = 16,
		URG = 32,
		ECE = 64,
		CWR = 128,
		NS = 256
	};

	struct tcp_header_t {
		uint16_t src_port;
		uint16_t dst_port;
		uint32_t seq_num;
		uint32_t ack;

		uint8_t reserved : 4;
		uint8_t data_offset : 4;
		uint8_t flags;

		uint16_t window_size;
		uint16_t checksum;
		uint16_t urgent_ptr; //Only if the urgent flag is set

		uint32_t options;
	} __attribute__((packed));

	struct tcp_pseudo_header_t {
		uint32_t src_ip;
		uint32_t dst_ip;
		uint16_t protocol;
		uint16_t total_len;
	} __attribute__((packed));

	class TcpSocket;
	class TcpProvider;

	class TcpHandler {
		public:
			TcpHandler();
			~TcpHandler();

			virtual bool onTcpMessage(TcpSocket* socket, uint8_t* data, size_t size);
	};

	class TcpSocket {
		public:
			TcpSocket(TcpProvider *provider);
			~TcpSocket();

			virtual bool handleTcpMessage(uint8_t* data, size_t size);
			virtual void send(uint8_t* data, size_t size);
			virtual void disconnect();

			TcpSocketState state;
			uint16_t remotePort;
			uint32_t remoteIp;
			uint16_t localPort;
			uint32_t localIp;
			uint32_t seq_num;
			uint32_t ack_num;

			TcpProvider* provider;
			TcpHandler* handler;
	};

	class TcpProvider: public Ipv4Handler {
		public:
			struct tcp_bind_t {
				uint16_t port;
				TcpSocket* handler;
			};

			listv2<tcp_bind_t> binds;

			int free_port = 1024;

			TcpProvider(Ipv4Provider *ipv4Provider);
			~TcpProvider();

			virtual bool onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size);

			virtual TcpSocket* connect(uint32_t ip, uint16_t port);
			virtual TcpSocket* listen(uint16_t port);

			virtual void disconnect(TcpSocket* socket);

			virtual void send(TcpSocket* socket, uint8_t* data, size_t size, uint16_t flags = 0);

			virtual void bind(TcpSocket* socket, TcpHandler* handler);
	};
}