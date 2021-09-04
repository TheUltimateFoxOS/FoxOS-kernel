#include <net/icmp.h>
#include <driver/serial.h>
#include <driver/nic/nic.h>

using namespace net;

IcmpProvider::IcmpProvider(Ipv4Provider* ipv4) : Ipv4Handler(ipv4, 0x01) {

}

IcmpProvider::~IcmpProvider() {

}

bool IcmpProvider::onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size) {
	if (size < sizeof(icmp_message_t)) {
		return false;
	}

	icmp_message_t* icmp = (icmp_message_t*) payload;

	switch (icmp->type) {
		case 0:
			{
				// Echo reply
				driver::nic::ip_u ip;
				ip.ip = srcIP_BE;

				driver::global_serial_driver->printf("ICMP: Echo reply from %d.%d.%d.%d\n", ip.ip_p[0], ip.ip_p[1], ip.ip_p[2], ip.ip_p[3]);
				return false;
			}
			break;
		case 8:
			{
				// Echo request
				icmp->type = 0;
				icmp->checksum = 0;
				icmp->checksum = this->backend->checksum((uint16_t*) icmp, sizeof(icmp_message_t));
				this->send(srcIP_BE, (uint8_t*) icmp, sizeof(icmp_message_t));
				return false;
			}
			break;
	}

	return false;
}

void IcmpProvider::send_echo_request(uint32_t dstIP_BE) {
	icmp_message_t icmp = {
		.type = 8,
		.code = 0,
		.checksum = 0,
		.data = 0x3713
	};

	icmp.checksum = backend->checksum((uint16_t*) &icmp, sizeof(icmp_message_t));
	this->send(dstIP_BE, (uint8_t*) &icmp, sizeof(icmp_message_t));
}