#pragma once
#include <stdint.h>
#include <net/etherframe.h>
#include <net/arp.h>

namespace net {
	struct ipv4_message_t {
		uint8_t header_length : 4;
		uint8_t version : 4;
		uint8_t type_of_service;
		uint16_t total_length;

		uint16_t identification;
		uint16_t flags_and_fragment_offset;

		uint8_t time_to_live;
		uint8_t protocol;
		uint16_t header_checksum;

		uint32_t source_address;
		uint32_t destination_address;
	};

	class Ipv4Provider;

	class Ipv4Handler {
		public:
			Ipv4Provider* backend;
			uint8_t protocol;
		
			Ipv4Handler(Ipv4Provider* backend, uint8_t protocol);
			~Ipv4Handler();

			void send(uint32_t dest_ip_be, uint8_t* payload, uint32_t size);

			virtual bool onInternetProtocolReceived(uint32_t srcIP_BE, uint32_t dstIP_BE, uint8_t* payload, uint32_t size);
	};

	class Ipv4Provider : public net::EtherFrameHandler {
		public:
			list<Ipv4Handler*> handlers;
			AddressResolutionProtocol* arp;

			uint32_t gateway_ip_be;
			uint32_t subnet_mask_be; 

			Ipv4Provider(EtherFrameProvider* backend, AddressResolutionProtocol* arp, uint32_t gateway_ip_be, uint32_t subnet_mask_be);
			~Ipv4Provider();

			virtual bool onEtherFrameReceived(uint8_t* payload, uint32_t size);
			void send(uint32_t dest_ip_be, uint8_t protocol, uint8_t* payload, uint32_t size);
			uint16_t checksum(uint16_t* data, uint32_t size);
	};
}