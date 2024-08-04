//
// MFEEPROM.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "MFEEPROM.h"
#include <EEPROM.h>
#include "MFBoards.h"
#if defined(ARDUINO_ARCH_RP2040)
#include "Core1.h"
#endif
#if defined(ARDUINO_ARCH_ESP32)
#include "Core0.h"
extern TaskHandle_t Core0handle;
#endif

MFEEPROM::MFEEPROM() {}

void MFEEPROM::init(void)
{
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP32)
    EEPROM.begin(4096);
#endif
    _eepromLength = EEPROM.length();
}

uint16_t MFEEPROM::get_length(void)
{
    return _eepromLength;
}

uint8_t MFEEPROM::read_byte(uint16_t adr)
{
    if (adr >= _eepromLength) return 0;
    return EEPROM.read(adr);
}

bool MFEEPROM::write_byte(uint16_t adr, const uint8_t data)
{
    if (adr >= _eepromLength) return false;
    EEPROM.write(adr, data);
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP32)
    #if defined(USE_CORE=0)
        // vTaskDelete(core0handle);
    #endif
        EEPROM.commit();
    #if defined(USE_CORE=0)
        // xTaskCreatePinnedToCore(core0,"Graphics",10000,NULL,0,&Core0handle,0);
    #endif
#endif
    return true;
}

// MFEEPROM.cpp