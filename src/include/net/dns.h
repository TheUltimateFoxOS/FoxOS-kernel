#pragma once
#include <net/udp.h>

namespace net {

	struct dnshdr_t {
		uint16_t id, opts, qdcount, ancount, nscount, arcount;
	};

	struct dns_question_t {
		uint16_t qtype, qclass;
	};

	struct dns_resource_t {
		uint16_t type, _class;
		uint32_t ttl;
		uint16_t data_len;
	} __attribute__((packed));

	struct grehdr_t {
		uint16_t opts, protocol;
	};

	class DomainNameServiceProvider: public UdpHandler {
		public:
			DomainNameServiceProvider(UdpSocket* socket);
			~DomainNameServiceProvider();

			void resolv_domain_to_hostname(char* dst_hostname, char* src_domain);

			void dns_request(char* name);

			virtual void onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size);

			UdpSocket* socket;
	};
}