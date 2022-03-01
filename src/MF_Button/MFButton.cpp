#include "MFButton.h"
#include "mobiflight.h"
#include "MFBitarray.h"

buttonEvent   MFButton::_handler = NULL;

MFButton::MFButton(uint8_t pin, const char * name, uint8_t arrayPosition)
{   
  _pin  = pin;
  _name = name;
  _state = 1;
  _arrayPosition = arrayPosition;       // save the x.te device for location in bitarray (buttonsRegistered)
  pinMode(_pin, INPUT_PULLUP);      // set pin to input
}

void MFButton::update()
{
    uint8_t newState = BitArray.getBit(_pin);
    if (newState!=_state) {     
      _state = newState;
      trigger(_state);
    }
}

void MFButton::readPin()            // copy the status of input pin to position of bitarray
{
    if (_pin <= MODULE_MAX_PINS)
        BitArray.setBit(_pin, digitalRead(_pin));
}

void MFButton::trigger(uint8_t state)
{
    (state==LOW) ? triggerOnPress() : triggerOnRelease();
}

void MFButton::triggerOnPress()
{
    if (_handler && _state==LOW) {
        (*_handler)(btnOnPress, _pin, _name);
    }
}

void MFButton::triggerOnRelease()
{
    if (_handler && _state==HIGH) {
        (*_handler)(btnOnRelease, _pin, _name);
    }
}

void MFButton::attachHandler(buttonEvent newHandler)
{
  _handler = newHandler;
}