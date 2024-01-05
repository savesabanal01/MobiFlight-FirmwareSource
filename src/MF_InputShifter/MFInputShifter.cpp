//
// MFInputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFInputShifter.h"
#include "allocateMem.h"

inputShifterEvent MFInputShifter::_inputHandler = NULL;

MFInputShifter::MFInputShifter()
{
    _initialized = false;
}

// Registers a new input shifter and configures the clock, data and latch pins as well
// as the number of modules to read from.
bool MFInputShifter::attach(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t moduleCount, const char *name)
{
    _latchPin    = latchPin;
    _clockPin    = clockPin;
    _dataPin     = dataPin;
    _name        = name;
    _moduleCount = moduleCount;

    pinMode(_latchPin, OUTPUT);
    pinMode(_clockPin, OUTPUT);
    pinMode(_dataPin, INPUT);

    if (!FitInMemory(sizeof(uint8_t) * _moduleCount))
        return false;
    
    _lastState = new (allocateMemory(sizeof(uint8_t) * _moduleCount)) uint8_t;
    for (uint8_t i = 0; i < _moduleCount; i++) {
        _lastState[i] = 0;
    }

    if (!FitInMemory(sizeof(uint8_t) * _moduleCount))
        return false;
    
    _currentState = new (allocateMemory(sizeof(uint8_t) * _moduleCount)) uint8_t;
    for (uint8_t i = 0; i < _moduleCount; i++) {
        _currentState[i] = 0;
    }

    _initialized = true;

    // And now initialize all buttons with the actual status
    poll();
    for (uint8_t module = 0; module < _moduleCount; module++) {
        _lastState[module] = _currentState[module];
    }
    return true;
}

// Reads the values from the attached modules, compares them to the previously
// read values, and calls the registered event handler for any inputs that
// changed from the previously read state.
void MFInputShifter::update()
{
// poll() is done in loop() or ISR, not here anymore
    for (uint8_t module = 0; module < _moduleCount; module++) {
        // If an input changed on the current module from the last time it was read
        // then hand it off to figure out which bits specifically changed.
        if (_currentState[module] != _lastState[module]) {
            detectChanges(_lastState[module], _currentState[module], module);
            _lastState[module] = _currentState[module];
        }
    }
}

void MFInputShifter::poll()
{
    digitalWrite(_clockPin, HIGH); // Preset clock to retrieve first bit
    digitalWrite(_latchPin, HIGH); // Disable input latching and enable shifting
    for (uint8_t module = 0; module < _moduleCount; module++) {
        _currentState[module] = shiftIn(_dataPin, _clockPin, MSBFIRST);
    }
    digitalWrite(_latchPin, LOW); // disable shifting and enable input latching
}

// Detects changes between the current state and the previously saved state
// of a byte's worth of input.
void MFInputShifter::detectChanges(uint8_t lastState, uint8_t currentState, uint8_t module)
{
    for (uint8_t i = 0; i < 8; i++) {
        // If last and current input state for the bit are different
        // then the input changed and the handler for the bit needs to fire
        if ((lastState & 1) ^ (currentState & 1)) {
            // When triggering event the pin is the actual pin on the chip offset by 8 bits for each
            // module beyond the first that it's on. The state of the trigger is the bit currently
            // in position 0 of currentState.
            trigger(i + (module * 8), currentState & 1);
        }

        lastState    = lastState >> 1;
        currentState = currentState >> 1;
    }
}

// Reads the current state for all connected modules then fires
// release events for every released button followed by
// press events for every pressed button.
void MFInputShifter::retrigger()
{
    uint8_t state;

    poll();

    // Trigger all the released buttons
    for (int module = 0; module < _moduleCount; module++) {
        state = _lastState[module];
        for (uint8_t i = 0; i < 8; i++) {
            // Only trigger if the button is in the off position
            if (state & 1) {
                trigger(i + (module * 8), HIGH);
            }
            state = state >> 1;
        }
    }

    // Trigger all the pressed buttons
    for (int module = 0; module < _moduleCount; module++) {
        state = _lastState[module];
        for (uint8_t i = 0; i < 8; i++) {
            // Only trigger if the button is in the on position
            if (!(state & 1)) {
                trigger(i + (module * 8), LOW);
            }

            state = state >> 1;
        }
    }
}

// Triggers the event handler for the associated input shift register pin,
// if a handler is registered.
void MFInputShifter::trigger(uint8_t pin, bool state)
{
    if (!_inputHandler) return;
    (*_inputHandler)((state == LOW ? inputShifterOnPress : inputShifterOnRelease), pin, _name);
}

// Attaches a new event handler for the specified event.
void MFInputShifter::attachHandler(inputShifterEvent newHandler)
{
    _inputHandler = newHandler;
}

void MFInputShifter::detach()
{
    _initialized = false;
}

// MFInputShifter.cpp
