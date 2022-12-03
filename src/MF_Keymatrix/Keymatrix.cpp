//
// KeyMatrix.cpp
//
// (C) MobiFlight Project 2022
//

#include <Arduino.h>
#include "MFKeymatrix.h"
#include "allocateMem.h"
#include "mobiflight.h"
#include "commandmessenger.h"
#include "KeyMatrix.h"
#include "MFBoards.h"

namespace Keymatrix
{
    MFKeymatrix *keymatrix[MAX_KEYMATRIX];
    uint8_t      keymatrixRegistered = 0;

    void handlerKeyMatrixOnChange(uint8_t eventId, uint8_t pin, const char *name)
    {
        cmdMessenger.sendCmdStart(kKeyMatrixChange);
        cmdMessenger.sendCmdArg(name);
        cmdMessenger.sendCmdArg(pin);
        cmdMessenger.sendCmdArg(eventId);
        cmdMessenger.sendCmdEnd();
    };

    void Add(uint8_t columnCount, uint8_t *columnPins, uint8_t rowCount, uint8_t *rowPins, const char *name)
    {
        if (keymatrixRegistered == MAX_KEYMATRIX)
            return;
        if (!FitInMemory(sizeof(MFKeymatrix))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("KeyMatrix does not fit in Memory"));
            return;
        }
        keymatrix[keymatrixRegistered] = new (allocateMemory(sizeof(MFKeymatrix))) MFKeymatrix(columnCount, columnPins, rowCount, rowPins, name);
        keymatrix[keymatrixRegistered]->init();
        MFKeymatrix::attachHandler(handlerKeyMatrixOnChange);
        keymatrixRegistered++;
    }

    void Clear()
    {
        for (int i = 0; i != keymatrixRegistered; i++) {
            keymatrix[i]->detach();
        }
        keymatrixRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared KeyMatrix"));
#endif
    }

    void read()
    {
        for (int i = 0; i != keymatrixRegistered; i++) {
            keymatrix[i]->update();
        }
    }
}
