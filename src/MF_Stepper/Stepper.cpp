//
// Stepper.cpp
//
// (C) MobiFlight Project 2022
//

#include "mobiflight.h"
#include "MFStepper.h"
#include "Stepper.h"
#if defined(STEPPER_ON_2ND_CORE)
#include <FreeRTOS.h>
#endif

namespace Stepper
{
    MFStepper *steppers;
    uint8_t    steppersRegistered = 0;
    uint8_t    maxSteppers        = 0;
#if defined(STEPPER_ON_2ND_CORE)
    enum {
        FUNC_MOVETO = 1,
        FUNC_ZETZERO,
        FUNC_SPEEDACCEL
    };
#endif

    bool setupArray(uint16_t count)
    {
        if (!FitInMemory(sizeof(MFStepper) * count))
            return false;
        steppers    = new (allocateMemory(sizeof(MFStepper) * count)) MFStepper;
        maxSteppers = count;
        return true;
    }

    void Add(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t btnPin1, uint8_t mode, int8_t backlash, bool deactivateOutput)
    {
        if (steppersRegistered == maxSteppers)
            return;
        steppers[steppersRegistered] = MFStepper();
        steppers[steppersRegistered].attach(pin1, pin2, pin3, pin4, btnPin1, mode, backlash, deactivateOutput);

        if (btnPin1 > 0) {
            // this triggers the auto reset if we need to reset
            steppers[steppersRegistered].reset();
        }

        // all set
        steppersRegistered++;

#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added stepper"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < steppersRegistered; i++) {
            steppers[i].detach();
        }
        steppersRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared steppers"));
#endif
    }

    void OnSet()
    {
        uint8_t stepper = (uint8_t)cmdMessenger.readInt16Arg();
        long    newPos  = cmdMessenger.readInt32Arg();

        if (stepper >= steppersRegistered)
            return;
#if defined(STEPPER_ON_2ND_CORE)
        // wait for 2nd core
        rp2040.fifo.pop();
        rp2040.fifo.push(FUNC_MOVETO);
        rp2040.fifo.push(stepper);
        rp2040.fifo.push(newPos);
        rp2040.fifo.push(false);
#else
        steppers[stepper].moveTo(newPos);
#endif
    }

    void OnReset()
    {
        uint8_t stepper = (uint8_t)cmdMessenger.readInt16Arg();

        if (stepper >= steppersRegistered)
            return;
        steppers[stepper].reset();
    }

    void OnSetZero()
    {
        uint8_t stepper = (uint8_t)cmdMessenger.readInt16Arg();

        if (stepper >= steppersRegistered)
            return;
#if defined(STEPPER_ON_2ND_CORE)
        // wait for 2nd core
        rp2040.fifo.pop();
        rp2040.fifo.push(FUNC_ZETZERO);
        rp2040.fifo.push(stepper);
        rp2040.fifo.push(false);
        rp2040.fifo.push(false);
#else
        steppers[stepper].setZero();
#endif
    }

    void OnSetSpeedAccel()
    {
        uint8_t  stepper  = (uint8_t)cmdMessenger.readInt16Arg();
        uint16_t maxSpeed = cmdMessenger.readInt16Arg();
        uint16_t maxAccel = cmdMessenger.readInt16Arg();

        if (stepper >= steppersRegistered)
            return;
#if defined(STEPPER_ON_2ND_CORE)
        rp2040.fifo.pop(); // wait for 2nd core
        rp2040.fifo.push(FUNC_SPEEDACCEL);
        rp2040.fifo.push(stepper);
        rp2040.fifo.push(maxSpeed);
        rp2040.fifo.push(maxAccel);
#else
        steppers[stepper].setMaxSpeed(maxSpeed);
        steppers[stepper].setAcceleration(maxAccel);
#endif
    }

    void update()
    {
#if !defined(STEPPER_ON_2ND_CORE)
        for (uint8_t i = 0; i < steppersRegistered; i++) {
            steppers[i].update();
        }
#endif
    }

    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < steppersRegistered; ++i) {
            steppers[i].powerSavingMode(state);
        }
    }

} // namespace

#if defined(STEPPER_ON_2ND_CORE)
/* **********************************************************************************
    This will run the set() function from the custom device on the 2nd core
    Be aware NOT to use the function calls from the Pico SDK!
    Only use the functions from the used framework from EarlePhilHower
    If you mix them up it will give undefined behaviour and strange effects
    see https://arduino-pico.readthedocs.io/en/latest/multicore.html
********************************************************************************** */
void setup1()
{
    rp2040.fifo.push(true); // inform core 1 to be ready
}

void loop1()
{
    uint8_t  command, stepper;
    int32_t param1, param2;

    while (1) {
        for (uint8_t i = 0; i < Stepper::steppersRegistered; ++i) {
            Stepper::steppers[i].update();
            if (rp2040.fifo.available()) {
                command = (uint8_t)rp2040.fifo.pop();
                stepper = (uint8_t)rp2040.fifo.pop();
                param1  = (int32_t)rp2040.fifo.pop();
                param2  = (int32_t)rp2040.fifo.pop();
                if (command == Stepper::FUNC_MOVETO) {
                    Stepper::steppers[stepper].moveTo(param1);
                } else if (command == Stepper::FUNC_ZETZERO) {
                    Stepper::steppers[stepper].setZero();
                } else if (command == Stepper::FUNC_SPEEDACCEL) {
                    Stepper::steppers[stepper].setMaxSpeed(param1);
                    Stepper::steppers[stepper].setAcceleration(param2);
                }
                rp2040.fifo.push(true); // inform core 1 to be ready for next command
            }
        }
    }
}
#endif

// Stepper.cpp
