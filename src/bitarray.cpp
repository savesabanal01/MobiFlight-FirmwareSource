#include <Arduino.h>
#include "bitarray.h"
#include "MFBoards.h"


uint8_t bitarray[MAX_KEYMATRIX * 8] = {0};

void setBit(uint8_t _position)
{
    uint8_t row = _position / 8;
    uint8_t _bit = _position % 8;
Serial.print("row: "); Serial.println(row);
Serial.print("Bit: "); Serial.println(_position);
    bitarray[row] |= 1 << _position;
}

