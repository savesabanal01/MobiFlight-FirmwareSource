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

    template <typename T>
    bool get(uint16_t adr, T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        EEPROM.get(adr, t);
        return true;
    }

    template <typename T>
    bool get(uint16_t adr, T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        uint8_t *ptr = (uint8_t*) &t;
        for (uint16_t i = 0; i < len; i++) {
            *ptr++ = EEPROM.read(adr + i);
        }
        return true;
    }

    template <typename T>
    const bool put(uint16_t adr, const T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        EEPROM.put(adr, t);
        return true;
    }

    template <typename T>
    const bool put(uint16_t adr, const T &t, uint16_t len)
    {
        if (adr + len > _eepromLength) return false;
        for (uint16_t i = 0; i < len; i++) {
            EEPROM.put(adr + i, t[i]);
        }
        return true;
    }

    template <typename T = char>
    const T read(uint16_t adr)
    {
        if (adr >= _eepromLength) return 0;
        return (T)EEPROM.read(adr);
    }

    template <typename T>
    const bool write(uint16_t adr, const T &t)
    {
        if (adr >= _eepromLength) return false;
        const uint8_t *ptr = (const uint8_t *)&t;
        EEPROM.put(adr, *ptr);
        return true;
    }
};

// MFEEPROM.h