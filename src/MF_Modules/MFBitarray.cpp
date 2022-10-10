#include <Arduino.h>
#include "MFBitarray.h"


MFBitArray::MFBitArray()
{
}

MFBitArray::~MFBitArray()
{
}

void MFBitArray::setBit(uint8_t _position, bool state)
{
    if (state)
        setBit(_position);
    else
        clearBit(_position);
}

void MFBitArray::setBit(uint8_t _position)
{
    if (_position/8 > sizeof(_bitarray))
        return;
    _bitarray[_position / 8] |= 1 << (_position % 8);
}

void MFBitArray::clearBit(uint8_t _position)
{
    if (_position/8 > sizeof(_bitarray))
        return;
    _bitarray[_position / 8] &= !(1 << (_position % 8));
}

bool MFBitArray::getBit(uint8_t _position)
{
    if (_position/8 > sizeof(_bitarray))
        return false;
    if (_bitarray[_position / 8] & (1 << (_position % 8)))
        return true;
    return false;
}
