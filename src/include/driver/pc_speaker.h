#include <driver/driver.h>

namespace driver {

	class PcSpeakerDriver : public Driver {
		public:
			PcSpeakerDriver();
			~PcSpeakerDriver();

			void turn_on();
			void turn_off();

			void set_frequency(uint16_t frequency);
			void play(uint16_t frequency);

			void play_note(uint8_t note); // (octave << 4) | note

			virtual void activate();
			virtual bool is_presend();
			virtual char* get_name();
	};
}