#pragma once

#define DRIVER_NUM 256

namespace driver {

	class Driver {
		public:
			Driver();
			~Driver();

			virtual void activate();
			virtual bool is_presend();
	};

	class DriverManager {
		private:
			Driver* drivers[DRIVER_NUM];
			int num_drivers;
			
		public:
			DriverManager();
			void add_driver(Driver* driver);
			void activate_driver(bool force, Driver* driver);
			void activate_all(bool force);
	};

	extern DriverManager* global_driver_manager;
}