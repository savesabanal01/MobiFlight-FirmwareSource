//
// allocatemem.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"

#ifdef ARDUINO_ARCH_RP2040
uint16_t    deviceBuffer[MF_MAX_DEVICEMEM] = {0};
#else
uint8_t     deviceBuffer[MF_MAX_DEVICEMEM] = {0};
#endif
uint16_t nextPointer                    = 0;

#ifdef ARDUINO_ARCH_RP2040
uint16_t    *allocateMemory(uint8_t size)
#else
uint8_t     *allocateMemory(uint8_t size)
#endif
{
    uint16_t actualPointer = nextPointer;
    nextPointer            = actualPointer + size;
    if (nextPointer >= MF_MAX_DEVICEMEM) {
        cmdMessenger.sendCmd(kStatus, F("DeviceBuffer Overflow!"));
        return nullptr;
    }
#ifdef DEBUG2CMDMESSENGER
    cmdMessenger.sendCmdStart(kDebug);
    cmdMessenger.sendCmdArg(F("BufferUsage"));
    cmdMessenger.sendCmdArg(nextPointer);
    cmdMessenger.sendCmdEnd();
#endif
    return &deviceBuffer[actualPointer];
}

void ClearMemory()
{
    nextPointer = 0;
}

uint16_t GetAvailableMemory()
{
    return MF_MAX_DEVICEMEM - nextPointer;
}

bool FitInMemory(uint8_t size)
{
    if (nextPointer + size > MF_MAX_DEVICEMEM)
        return false;
    return true;
}

// allocatemem.cpp
