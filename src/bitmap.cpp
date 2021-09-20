#include <bitmap.h>

bool Bitmap::operator[](uint64_t index){
	return get(index);
}

//#Bitmap::get-doc: Get a bit in the bitmap.
bool Bitmap::get(uint64_t index) {
	if (index > size * 8) return false;

	uint64_t byteIndex = index / 8;
	uint8_t bitIndex = index % 8;
	uint8_t bitIndexer = 0b10000000 >> bitIndex;
	if ((buffer[byteIndex] & bitIndexer) > 0){
		return true;
	}
	
	return false;
}

//#Bitmap::set-doc: Set a bit in the bitmap.
bool Bitmap::set(uint64_t index, bool value){
	if (index > size * 8) return false;

	uint64_t byteIndex = index / 8;
	uint8_t bitIndex = index % 8;
	uint8_t bitIndexer = 0b10000000 >> bitIndex;
	buffer[byteIndex] &= ~bitIndexer;
	if (value){
		buffer[byteIndex] |= bitIndexer;
	}

	return true;
}