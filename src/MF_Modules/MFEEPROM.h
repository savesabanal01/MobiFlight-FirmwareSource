//
// MFEEPROM.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <stdint.h>

class MFEEPROM
{

public:
    MFEEPROM();
    void     init(void);
    uint16_t get_length(void);
    bool     read_block(uint16_t addr, char data[], uint16_t len);
    bool     write_block(uint16_t addr, char data[], uint16_t len);
    char     read_char(uint16_t adr);
    bool     write_byte(uint16_t adr, char data);

    template <typename T>
    bool read_block(int adr, T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        uint8_t *ptr = (uint8_t *)&t;
        for (uint16_t i = 0; i < sizeof(T); i++) {
            *ptr++ = read_char(adr + i);
        }
        return true;
    }

    template <typename T>
    const bool write_block(int adr, const T &t)
    {
        if (adr + sizeof(T) > _eepromLength) return false;
        const uint8_t *ptr = (const uint8_t *)&t;
        for (uint16_t i = 0; i < sizeof(T); i++) {
            write_byte(adr + i, *ptr++);
        }
        return true;
    }

private:
    uint16_t _eepromLength = 0;
};

// MFEEPROM.h