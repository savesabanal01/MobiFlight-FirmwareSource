//
// MFEEPROM.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "MFEEPROM.h"
#include <EEPROM.h>
#include "MFBoards.h"

MFEEPROM::MFEEPROM()
{
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(EEPROM_SIZE);
#endif
    eepromLength = EEPROM.length();
}

uint16_t MFEEPROM::get_length(void)
{
    return eepromLength;
}

void MFEEPROM::read_block(uint16_t adr, char data[], uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        data[i] = read_char(adr + i);
    }
}

void MFEEPROM::write_block(uint16_t adr, char data[], uint16_t len)
{
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i < len; i++) {
        EEPROM.put(adr + i, data[i]);
    }
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif
}

char MFEEPROM::read_char(uint16_t adr)
{
    if (adr >= eepromLength) return 0;
    return EEPROM.read(adr);
}

void MFEEPROM::write_byte(uint16_t adr, char data)
{
    if (adr >= eepromLength) return;
    EEPROM.put(adr, data);
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.commit();
#endif
}

// MFEEPROM.cpp