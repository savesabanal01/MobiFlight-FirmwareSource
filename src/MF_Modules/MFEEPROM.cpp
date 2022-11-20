//
// MFEEPROM.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "MFEEPROM.h"

MFEEPROM::MFEEPROM() {}

void MFEEPROM::init(void)
{
    _eepromLength = EEPROM.length();
}

uint16_t MFEEPROM::get_length(void)
{
    return _eepromLength;
}

uint8_t MFEEPROM::read(uint16_t adr)
{
    if (adr >= _eepromLength) return 0;
    return EEPROM.read(adr);
}

bool MFEEPROM::write(uint16_t adr, const uint8_t data)
{
    if (adr >= _eepromLength) return false;
    EEPROM.write(adr, data);
    return true;
}

// MFEEPROM.cpp