#pragma once

#include "MFBoards.h"

#define SIZE_BITARRAY   (MAX_KEYMATRIX*8)+((MAX_BUTTONS/8)+1)
class MFBitArray
{
private:
    uint8_t _bitarray[SIZE_BITARRAY] = {0};
public:
    MFBitArray();
    ~MFBitArray();
    void setBit(uint8_t _position, bool state);
    void setBit(uint8_t _position);
    void clearBit(uint8_t _position);
    bool getBit(uint8_t _position);
};
