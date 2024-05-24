//
// MFEEPROM.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <EEPROM.h>

class MFEEPROM
{
private:
    uint16_t _eepromLength = 0;

public:
    MFEEPROM();
    void     init(void);
    uint16_t get_length(void);
    uint8_t read_byte(uint16_t adr);
    bool write_byte(uint16_t adr, const uint8_t data);
    void commit();

    template <typename T>
    bool read_block(uint16_t adr, T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
#if defined(ARDUINO_ARCH_STM32)
        uint8_t *ptr = (uint8_t *) &t;
        for (int count = sizeof(T) ; count ; --count) {
            *ptr++ = eeprom_buffered_read_byte(adr + count);
        }
#else
        EEPROM.get(adr, t);
#endif
        return true;
    }

    template <typename T>
    bool read_block(uint16_t adr, T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        uint8_t *ptr = (uint8_t*) &t;
        for (uint16_t i = 0; i < len; i++) {
#if defined(ARDUINO_ARCH_STM32)
            read_block(adr + i * sizeof(T), t[i]);
#else
            *ptr++ = EEPROM.read(adr + i);
#endif
        }
        return true;
    }

    template <typename T>
    const bool write_block(uint16_t adr, const T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
#if defined(ARDUINO_ARCH_STM32)
        const uint8_t *ptr = (const uint8_t *) &t;
//Serial.print("Writing to buffer: ");
        for (int count = sizeof(T) ; count ; --count) {
//Serial.print((char)*ptr);
            eeprom_buffered_write_byte(adr + count, *ptr++);
        }
//Serial.println();
#else
        EEPROM.put(adr, t);
#endif
        return true;
    }

    template <typename T>
    const bool write_block(uint16_t adr, const T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        for (uint16_t i = 0; i < len; i++) {
#if defined(ARDUINO_ARCH_STM32)
            write_block(adr + i * sizeof(T), t[i]);
#else
            EEPROM.put(adr + i, t[i]);
#endif
        }
        return true;
    }
};

// MFEEPROM.h