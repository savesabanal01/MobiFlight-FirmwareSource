//
// MFButton.cpp
//
// (C) MobiFlight Project 2022
//
#include "MFButton.h"
#include "DigInMux.h"

buttonEvent MFButton::_handler = NULL;

MFButton::MFButton()
{
    _initialized = false;
}

void MFButton::attach(uint8_t pin, const char *name)
{

    _name   = name;
    if (pin < 100)
    {
#ifdef USE_FAST_IO
        _pin.Port = portInputRegister(digitalPinToPort(pin));
        _pin.Mask = digitalPinToBitMask(pin);
#else
        _pin   = pin;
#endif
        pinMode(pin, INPUT_PULLUP);    // set pin to input
        _state = digitalRead(pin);     // initialize on actual status
        _useMUX = 0;
    } else
    {
        _pinMux = pin - 100;
        _useMUX = (_pinMux >> 4) + 1;
        _state = DigInMux::readPin(_useMUX - 1, _pinMux);
    }
    
    _initialized = true;
}

void MFButton::update()
{
    if (!_initialized)
        return;
    
    uint8_t newState = 0;

    if (_useMUX)
        newState = DigInMux::readPin(_useMUX - 1, _pinMux);
    else
        newState = DIGITALREAD(_pin);

    if (newState != _state) {
        _state = newState;
        trigger(_state);
    }
}

void MFButton::trigger(uint8_t state)
{
    (state == LOW) ? triggerOnPress() : triggerOnRelease();
}

void MFButton::triggerOnPress()
{
    if (!_initialized)
        return;
    if (_handler && _state == LOW) {
        (*_handler)(btnOnPress, _name);
    }
}

void MFButton::triggerOnRelease()
{
    if (!_initialized)
        return;
    if (_handler && _state == HIGH) {
        (*_handler)(btnOnRelease, _name);
    }
}

void MFButton::attachHandler(buttonEvent newHandler)
{
    _handler = newHandler;
}

// MFButton.cpp