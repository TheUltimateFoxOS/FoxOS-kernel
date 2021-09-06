#pragma once
#include <stdint.h>
#include <driver/nic/nic.h>
#include <net/listv2.h>

#define MAX_ETHER_FRAME_HANDLERS 65535

namespace net {
	struct ether_frame_header_t {
		uint64_t dest_mac_be : 48;
		uint64_t src_mac_be : 48;
		uint16_t ether_type_be;
	} __attribute__((packed));

	typedef uint32_t ether_frame_footer_t;

	class EtherFrameProvider;

	class EtherFrameHandler {
		public:
			EtherFrameProvider* backend;
			uint16_t ether_type_be;
		
			EtherFrameHandler(EtherFrameProvider* backend, uint16_t ether_type);
			~EtherFrameHandler();

			virtual bool onEtherFrameReceived(uint8_t* payload, uint32_t size);
			void send(uint64_t dest_mac_be, uint8_t* payload, uint32_t size);
	};

	class EtherFrameProvider : public driver::nic::NicDataManager {
		public:
			listv2<EtherFrameHandler*> handlers;

			EtherFrameProvider(int nic_id);
			~EtherFrameProvider();

			virtual bool recv(uint8_t* data, int32_t size) override;
			void send_f(uint64_t dest_mac_be, uint16_t ether_type_be, uint8_t* payload, uint32_t size);

	};
}