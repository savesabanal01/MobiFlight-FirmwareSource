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
    bool     read_block(uint16_t addr, uint8_t data[], uint16_t len);
    bool     read_block(uint16_t addr, char data[], uint16_t len);
    bool     write_block(uint16_t addr, uint8_t data[], uint16_t len);
    bool     write_block(uint16_t addr, char data[], uint16_t len);
    uint8_t  read_byte(uint16_t adr);
    bool     write_byte(uint16_t adr, char data);
    /*
        template <typename T>
        bool read_block(uint16_t adr, T &t)
        {
            if (adr + sizeof(T) > _eepromLength) return false;
            char *ptr = (uint8_t *)&t;
            for (uint16_t i = 0; i < sizeof(T); i++) {
                *ptr++ =EEPROM.read(adr + i);
            }
            return true;
        }

        template <typename T>
        bool read_block(uint16_t adr, T &t, uint16_t len)
        {
            if (adr + len > _eepromLength) return false;
            uint8_t *ptr = (uint8_t *)&t;
            for (uint16_t i = 0; i < len; i++) {
                *ptr++ =EEPROM.read(adr + i);
            }
            return true;
        }

        template <typename T>
        const bool write_block(uint16_t adr, const T &t)
        {
            if (adr + sizeof(T) > _eepromLength) return false;
            const uint8_t *ptr = (const uint8_t *)&t;
            for (uint16_t i = 0; i < sizeof(T); i++) {
                EEPROM.put(adr + i, *ptr++);
            }
            return true;
        }

        template <typename T>
        const bool write_block(uint16_t adr, const T &t, uint16_t len)
        {
            if (adr + len > _eepromLength) return false;
            const uint8_t *ptr = (const uint8_t *)&t;
            for (uint16_t i = 0; i < len; i++) {
                EEPROM.put(adr + i, *ptr++);
            }
            return true;
        }

        template <typename T>
        T read_byte(uint16_t adr)
        {
            if (adr >= _eepromLength) return 0;
            return EEPROM.read(adr);
        }

        template <typename T>
        bool write_byte(uint16_t adr, const T &t)
        {
            if (adr >= _eepromLength) return false;
            const uint8_t *ptr = (const uint8_t *)&t;
            EEPROM.put(adr, *ptr);
            return true;
        }
    */

private:
    uint16_t _eepromLength = 0;
};

// MFEEPROM.h