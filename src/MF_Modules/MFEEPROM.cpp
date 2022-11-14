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

// MFEEPROM.cpp