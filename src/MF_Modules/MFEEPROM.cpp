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
    EEPROM.begin(EEPROM_SIZE);
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
#if defined(ARDUINO_ARCH_RP2040)
    // #########################################################################
    // Communication with Core1
    // see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
    // #########################################################################
    multicore_fifo_push_blocking(CORE1_CMD_STOP);
    multicore_lockout_start_blocking();
    EEPROM.commit();
    multicore_lockout_end_blocking();
#endif
#if defined(ARDUINO_ARCH_ESP32) && defined(USE_CORE0)
    // vTaskDelete(core0handle);
    EEPROM.commit();
    // xTaskCreatePinnedToCore(core0,"Graphics",10000,NULL,0,&Core0handle,0);
#endif
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP32)
    EEPROM.commit();
#endif
    return true;
}

// MFEEPROM.cpp