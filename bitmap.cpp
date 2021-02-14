#include <bitmap.h>

bool Bitmap::operator[](uint64_t index){
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    if ((buffer[byteIndex] & bitIndexer) > 0){
        return true;
    }
    return false;
}

void Bitmap::set(uint64_t index, bool value){
    uint64_t byteIndex = index / 8;
    uint8_t bitIndex = index % 8;
    uint8_t bitIndexer = 0b10000000 >> bitIndex;
    buffer[byteIndex] &= ~bitIndexer;
    if (value){
        buffer[byteIndex] |= bitIndexer;
    }
}