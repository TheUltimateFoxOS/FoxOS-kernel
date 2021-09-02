#ifndef Nic_H
#define Nic_H

#include <stdint.h>

#define NIC_NUM 265

typedef void (*nic_recv_handler)(uint8_t* data, uint32_t len);

namespace driver {
	namespace nic {
		class Nic;
		class NicDataManager {
			public:
				NicDataManager(int nic_id);
				~NicDataManager();

				void send(uint8_t* data, int32_t len);
				virtual bool recv(uint8_t* data, int32_t len);

				Nic* nic;
				int nic_id;
		};
		class Nic {
			public:
				Nic();
				~Nic();
				virtual void send(uint8_t* data, int32_t len);

				virtual void register_nic_data_manager(NicDataManager* nic_data_manager);
				virtual uint64_t get_mac();

				NicDataManager* nic_data_manager;

		};

		class NicManager {
			private:
				Nic* Nics[NIC_NUM];
			public:
				int num_Nics;
				NicManager();

				Nic* get_nic(int nic_id);
				void add_Nic(Nic* Nic);
		};

		extern NicManager* global_nic_manager;
	}
}

#endif