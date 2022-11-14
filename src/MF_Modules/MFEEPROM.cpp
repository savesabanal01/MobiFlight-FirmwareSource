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
/*
bool MFEEPROM::read_block(uint16_t adr, uint8_t data[], uint16_t len)
{
    if (adr + len > _eepromLength) return false;
    for (uint16_t i = 0; i < len; i++) {
        data[i] = EEPROM.read(adr + i);
    }
    return true;
}

bool MFEEPROM::read_block(uint16_t adr, char data[], uint16_t len)
{
    if (adr + len > _eepromLength) return false;
    for (uint16_t i = 0; i < len; i++) {
        data[i] = EEPROM.read(adr + i);
    }
    return true;
}

bool MFEEPROM::write_block(uint16_t adr, uint8_t data[], uint16_t len)
{
    if (adr + len > _eepromLength) return false;
    for (uint16_t i = 0; i < len; i++) {
        EEPROM.put(adr + i, data[i]);
    }
    return true;
}

bool MFEEPROM::write_block(uint16_t adr, char data[], uint16_t len)
{
    if (adr + len > _eepromLength) return false;
    for (uint16_t i = 0; i < len; i++) {
        EEPROM.put(adr + i, data[i]);
    }
    return true;
}

uint8_t MFEEPROM::read_byte(uint16_t adr)
{
    if (adr >= _eepromLength) return 0;
    return EEPROM.read(adr);
}

bool MFEEPROM::write_byte(uint16_t adr, char data)
{
    if (adr >= _eepromLength) return false;
    EEPROM.put(adr, data);
    return true;
}
*/
// MFEEPROM.cpp