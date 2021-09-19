#include <driver/pc_speaker.h>
#include <scheduling/pit/pit.h>
#include <scheduling/hpet/hpet.h>
#include <port.h>

using namespace driver;

//#PcSpeakerDriver::PcSpeakerDriver-doc: Empty constructor.
PcSpeakerDriver::PcSpeakerDriver() {
}

//#PcSpeakerDriver::~PcSpeakerDriver-doc: Empty destructor.
PcSpeakerDriver::~PcSpeakerDriver() {
}

//#PcSpeakerDriver::activate-doc: This function is used to override the default function from the Driver base class.
void PcSpeakerDriver::activate() {
	// that just takes more time too boot and is useless
	/*play_note((1 << 4) | 1);
	if (hpet::is_available()) {
		hpet::sleep_d(0.5);
	} else {
		PIT::sleep_d(0.5);
	}
	turn_off();*/
}

//#PcSpeakerDriver::is_presend-doc: This function is used to override the default function from the Driver base class.
bool PcSpeakerDriver::is_presend() {
	return true;
}

//#PcSpeakerDriver::get_name-doc: This function is used to override the default function from the Driver base class.
char* PcSpeakerDriver::get_name() {
	return (char*) "pc speaker";
}

//#PcSpeakerDriver::turn_on-doc: This function is used to turn on the pc speaker.
void PcSpeakerDriver::turn_on() {
	outb(0x61, (inb(0x61) | 3));
}

//#PcSpeakerDriver::turn_off-doc: This function is used to turn off the pc speaker.
void PcSpeakerDriver::turn_off() {
	outb(0x61, (inb(0x61) & 0xFC));
}

//#PcSpeakerDriver::set_frequency-doc: This function is used to set the frequency of the pc speaker.
void PcSpeakerDriver::set_frequency(uint16_t frequency) {
	outb(0x43, 0xB6);
	outb(0x42, (uint8_t) frequency);
	outb(0x42, (uint8_t) (frequency >> 8));
}

//#PcSpeakerDriver::play-doc: This function is used to set the frequency of the pc speaker and then automatically starts playing.
void PcSpeakerDriver::play(uint16_t frequency) {
	set_frequency(frequency);
	turn_on();
}


uint16_t notes[7][12] = {
	{ 36485, 34437, 32505, 30680, 28958, 27333, 25799, 24351, 22984, 21694, 20477, 19327 },
	{ 18243, 17219, 16252, 15340, 14479, 13666, 12899, 12175, 11492, 10847, 10238, 9664 },
	{ 9121, 8609, 8126, 7670, 7240, 6833, 6450, 6088, 5746, 5424, 5119, 4832 },
	{ 4561, 4305, 4063, 3835, 3620, 3417, 3225, 3044, 2873, 2712, 2560, 2416 },
	{ 2280, 2152, 2032, 1918, 1810, 1708, 1612, 1522, 1437, 1356, 1280, 1208 },
	{ 1140, 1076, 1016, 959, 905, 854, 806, 761, 718, 678, 640, 604},
	{ 570, 538, 508, 479, 452, 427, 403, 380, 359, 339, 320, 302 }
};

//#PcSpeakerDriver::play_note-doc: This function is used to play a note. The note is encoded with the following format: (octave << 4) | note.
void PcSpeakerDriver::play_note(uint8_t note) {
	play(notes[note >> 4][note & 0xf]);
}
