//
// allocatemem.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <new>

#ifdef ARDUINO_ARCH_RP2040
uint16_t    *allocateMemory(uint8_t size);
#else
uint8_t     *allocateMemory(uint8_t size);
#endif
void        ClearMemory();
uint16_t    GetAvailableMemory();
bool        FitInMemory(uint8_t size);

// allocatemem.h
