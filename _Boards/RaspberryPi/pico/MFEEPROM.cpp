#include <Arduino.h>
#include "MFBoards.h"
#include "MFEEPROM.h"
#include <EEPROM.h>

MFEEPROM::MFEEPROM() {}

uint16_t MFEEPROM::get_length(void) {
    return eepromLength;
}

void MFEEPROM::read_block(uint16_t adr, char data[], uint16_t len) {
    for (uint16_t i = 0; i<len; i++) {
        data[i] = read_char(adr + i);
    }
}

void MFEEPROM::init() {
    EEPROM.begin(EEPROM_SIZE);
    eepromLength = EEPROM.length();
}

void MFEEPROM::write_block (uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        EEPROM.put(adr + i,data[i]);
    }
    EEPROM.commit();
}

char MFEEPROM::read_char(uint16_t adr) {
    if (adr >= eepromLength) return 0;
    return EEPROM.read(adr);
}

void MFEEPROM::write_byte (uint16_t adr, char data) {
    if (adr >= eepromLength) return;
    EEPROM.put(adr, data);
    EEPROM.commit();
}


/*

#if defined(ARDUINO_ARCH_AVR) || defined(CORE_TEENSY) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_STM32F1)

void eeprom_init(void) {
#if defined(ARDUINO_ARCH_RP2040)
    EEPROM.begin(EEPROM_SIZE);
#endif
# if !defined(ARDUINO_ARCH_STM32F1)
    eepromLength = EEPROM.length();
#else
    eepromLength = EEPROM_SIZE;
#endif
    return;
}

void eeprom_read_block(uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        data[i] = EEPROM.read(adr+i);
    }
}

void eeprom_write_block (uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
#if defined(ARDUINO_ARCH_STM32F1)
        EEPROM.write(adr + i,data[i]);
#else
        EEPROM.put(adr + i,data[i]);
#endif
    }
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP8266)
    EEPROM.commit();
#endif
}

char eeprom_read_char(uint16_t adr) {
    if (adr >= eepromLength) return 0;
    return EEPROM.read(adr);
}

void eeprom_write_byte (uint16_t adr, char data) {
    if (adr >= eepromLength) return;
#if defined(ARDUINO_ARCH_STM32F1)
    EEPROM.write(adr,data);
#else
    EEPROM.put(adr, data);
#endif
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP8266)
    EEPROM.commit();
#endif
}


#elif defined(ARDUINO_ARCH_STM32)

#if defined(DATA_EEPROM_BASE)
# warning  "STM32 w/o buffered API"

void eeprom_init(void) {
        EEPROM.init();
        eepromLength = EEPROM.length();
}

void eeprom_read_block(uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        data[i] = EEPROM.read(adr + i);
    }
}

void eeprom_write_block (uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        EEPROM.write(adr + i,data[i]);
    }
}

uint8_t eeprom_read_char(uint16_t adr, char data) {
    if (adr >= eepromLength) return 0;
    data = EEPROM.read(adr);
}

void eeprom_write_byte (uint16_t adr, char data) {
    if (adr >= eepromLength) return;
    EEPROM.write(adr, data);
}

#else

# warning  "STM32 with buffered API"

void eeprom_init(void) {
        eeprom_buffer_fill();
        eepromLength = EEPROM.length();
}

void eeprom_read_block(uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        data[i] = eeprom_buffered_read_byte(adr + i);
    }
}

void eeprom_write_block (uint16_t adr, char data[], uint16_t len) {
    if (adr + len >= eepromLength) return;
    for (uint16_t i = 0; i<len; i++) {
        eeprom_buffered_write_byte(adr + i,data[i]);
    }
    eeprom_buffer_flush();
}

char eeprom_read_char(uint16_t adr) {
    if (adr >= eepromLength) return 0;
    return eeprom_buffered_read_byte(adr);
}

void eeprom_write_byte (uint16_t adr, char data) {
    if (adr >= eepromLength) return;
    EEPROM.write(adr, data);
    eeprom_buffer_flush();
}
#endif

#endif
*/