#include <net/dns.h>
#include <string.h>
#include <driver/serial.h>
#include <driver/nic/nic.h>

using namespace net;

DomainNameServiceProvider::DomainNameServiceProvider(UdpSocket* socket) : results(100) {
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

	delete[] buffer;
}

uint32_t DomainNameServiceProvider::resolve(char* name) {
	char qname[128];
	resolv_domain_to_hostname(qname, name);

	list<dns_result_t>::node* n = results.find<char*>([](char* qname, list<dns_result_t>::node* n) {
		return strcmp(qname, n->data.name) == 0;
	}, qname);

	if (n != nullptr) {
		return n->data.ip;
	} else {
		wait_for_response = true;
		dns_request(name);

		int timeout = 10000000;
		while (true) {
			if (timeout-- <= 0) {
				return 0;
			}

			if (wait_for_response == false) {
				break;
			}
		}

		list<dns_result_t>::node* n = results.find<char*>([](char* qname, list<dns_result_t>::node* n) {
			return strcmp(qname, n->data.name) == 0;
		}, qname);

		if (n != nullptr) {
			return n->data.ip;
		} else {
			return 0;
		}
	}

	return 0;
}

// sort of hacky, but it works

void DomainNameServiceProvider::onUdpMessage(UdpSocket *socket, uint8_t* data, size_t size) {
	dnshdr_t* dns = (dnshdr_t*)data;
	char* qname = (char*)(data + sizeof(dnshdr_t));
	dns_question_t* question = (dns_question_t*)(data + sizeof(dnshdr_t) + strlen(qname) + 1);

	if (dns->id != 0xf00f) {
		return;
	}

	if (question->qtype != __builtin_bswap16(1)) {
		return;
	}

	if (question->qclass != __builtin_bswap16(1)) {
		return;
	}

	dns_resource_t* resource = (dns_resource_t*)(data + sizeof(dnshdr_t) + strlen(qname) + 1 + sizeof(dns_question_t) + sizeof(uint16_t));

	if (resource->type != __builtin_bswap16(1)) {
		return;
	}

	if (resource->_class != __builtin_bswap16(1)) {
		return;
	}

	uint32_t* ip = (uint32_t*)(data + sizeof(dnshdr_t) + strlen(qname) + 1 + sizeof(dns_question_t) + sizeof(uint16_t) + sizeof(dns_resource_t));

	driver::nic::ip_u resolved_ip;
	resolved_ip.ip = *ip;

	driver::global_serial_driver->printf("DNS: %s -> %d.%d.%d.%d\n", qname, resolved_ip.ip_p[0], resolved_ip.ip_p[1], resolved_ip.ip_p[2], resolved_ip.ip_p[3]);

	dns_result_t result;
	result.ip = *ip;
	strcpy(result.name, qname);

	results.add(result);

	wait_for_response = false;
}