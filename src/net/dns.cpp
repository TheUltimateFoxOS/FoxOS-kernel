#include <net/dns.h>
#include <string.h>
#include <driver/serial.h>

using namespace net;

DomainNameServiceProvider::DomainNameServiceProvider(UdpSocket* socket) {
	this->socket = socket;
}

DomainNameServiceProvider::~DomainNameServiceProvider() {

}

void DomainNameServiceProvider::resolv_domain_to_hostname(char* dst_hostname, char* src_domain) {
	int len = strlen(src_domain) + 1;
	char* lbl = dst_hostname;
	char* dst_pos = dst_hostname + 1;
	uint8_t curr_len = 0;

	while (len-- > 0) {
		char c = *src_domain++;

		if (c == '.' || c == 0) {
			*lbl = curr_len;
			lbl = dst_pos++;
			curr_len = 0;
		} else {
			curr_len++;
			*dst_pos++ = c;
		}
	}
	*dst_pos = 0;
}

void DomainNameServiceProvider::dns_request(char* name) {
	char* buffer = new char[2048];
	dnshdr_t* dns = (dnshdr_t*)buffer;
	char* qname = (char*)(buffer + sizeof(dnshdr_t));

	resolv_domain_to_hostname(qname, name);

	dns_question_t* question = (dns_question_t*)(buffer + sizeof(dnshdr_t) + strlen(qname) + 1);

	dns->id = 0xf00f;
	dns->opts = __builtin_bswap16(1 << 8);
	dns->qdcount = __builtin_bswap16(1);
	question->qtype = __builtin_bswap16(1);
	question->qclass = __builtin_bswap16(1);

	this->socket->send((uint8_t*) buffer, sizeof(dnshdr_t) + strlen(qname) + 1 + sizeof(dns_question_t));
}

void DomainNameServiceProvider::onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size) {

}