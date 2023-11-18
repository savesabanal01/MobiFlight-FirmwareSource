//
// MFButton.cpp
//
// (C) MobiFlight Project 2022
//
#include "MFButton.h"

buttonEvent MFButton::_handler = NULL;

MFButton::MFButton(uint8_t pin, const char *name)
{
    _pin  = pin;
    _name = name;
    pinMode(_pin, INPUT_PULLUP); // set pin to input
    _state = digitalRead(_pin);  // initialize on actual status
}

void MFButton::update()
{
    if (_newState != _state) {
        _state = _newState;
        trigger(_state);
    }
}

void MFButton::poll()
{
    _newState = (uint8_t)digitalRead(_pin);
}

void MFButton::trigger(uint8_t state)
{
    (state == LOW) ? triggerOnPress() : triggerOnRelease();
}

void MFButton::triggerOnPress()
{
    if (_handler && _state == LOW) {
        (*_handler)(btnOnPress, _name);
    }
}

void MFButton::triggerOnRelease()
{
    if (_handler && _state == HIGH) {
        (*_handler)(btnOnRelease, _name);
    }
}

void MFButton::attachHandler(buttonEvent newHandler)
{
    _handler = newHandler;
}

// MFButton.cpp
