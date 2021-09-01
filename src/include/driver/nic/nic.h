#ifndef Nic_H
#define Nic_H

#include <stdint.h>

#define NIC_NUM 265

typedef void (*nic_recv_handler)(uint8_t* data, uint32_t len);

namespace driver {
	namespace nic {
		class Nic {
			public:
				Nic();
				~Nic();
				virtual void send(uint8_t* data, int32_t len);
				void recv(nic_recv_handler handler);
			
				nic_recv_handler handler;
		};

		class NicManager {
			private:
				Nic* Nics[NIC_NUM];
			public:
				int num_Nics;
				NicManager();

				void send(int id, uint8_t* data, int32_t len);
				void recv(int id, nic_recv_handler handler);

				void add_Nic(Nic* Nic);
		};

		extern NicManager* global_nic_manager;
	}
}

#endif