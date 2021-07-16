#pragma once

#include <stdint.h>

#define DRIVER_NUM 256

namespace driver {

	class Driver {
		public:
			Driver();
			~Driver();

			virtual void activate();
			virtual bool is_presend();
			virtual char* get_name();
	};

	class DriverManager {
		private:
			Driver* drivers[DRIVER_NUM];
			int num_drivers;
			
		public:
			DriverManager();
			void add_driver(Driver* driver);
			void set_status(char* status, uint64_t color);
			void activate_driver(bool force, Driver* driver);
			void activate_all(bool force);

			Driver* find_driver_by_name(char* name);
	};

	extern DriverManager* global_driver_manager;
}