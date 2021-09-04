#pragma once
#include <net/ipv4.h>

namespace net {
	struct icmp_message_t {
		uint8_t type;
		uint8_t code;

		uint16_t checksum;
		uint32_t data;
	} __attribute__((packed));

	class IcmpProvider : public Ipv4Handler {
		public:
			IcmpProvider(Ipv4Provider* ipv4);
			~IcmpProvider();

			virtual bool onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size);
			void send_echo_request(uint32_t dstIP_BE);
	};
}