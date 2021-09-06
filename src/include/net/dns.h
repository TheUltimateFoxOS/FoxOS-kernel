#pragma once
#include <net/udp.h>
#include <net/listv2.h>

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

	struct dns_result_t {
		uint32_t ip;
		char name[64];
	};

	class DomainNameServiceProvider: public UdpHandler {
		public:
			DomainNameServiceProvider(UdpSocket* socket);
			~DomainNameServiceProvider();

			void resolv_domain_to_hostname(char* dst_hostname, char* src_domain);

			uint32_t resolve(char* name);
			void dns_request(char* name);

			virtual void onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size);
			
			UdpSocket* socket;
			list<dns_result_t> results;

			bool wait_for_response;
	};
}