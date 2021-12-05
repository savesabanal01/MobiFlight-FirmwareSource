// MFServo.cpp
//
// Copyright (C) 2013-2014

#include "MFServo.h"
#include "CmdMessenger.h"
extern CmdMessenger cmdMessenger;

void MFServo::moveTo(int absolute)
{
	int newValue = map(absolute, _mapRange[0], _mapRange[1], _mapRange[2], _mapRange[3]);
    if (_targetPos != newValue)
    {
			_targetPos = newValue;
			if (!_initialized) {
			  _servo.attach(_pin);
				_initialized = true;
			}
    }
}

void MFServo::update() {
	// after reaching final position
	// detach the servo to prevent continuous noise
    if (_currentPos == _targetPos) { 
		// detach(); 
		return; 
	}

	uint8_t step = abs(_currentPos - _targetPos);
	if (step > 20) {
		step = 5;
	} else {
		step = 1;
	}

    if (_currentPos > _targetPos) {
		_currentPos-= step;
	} else {
		_currentPos+= step;
	}
        
    _servo.write(_currentPos);
}

void MFServo::detach() { 
  if (_initialized) {
    _servo.detach(); 
    _initialized = false; 
  }
}

void MFServo::attach(uint8_t pin, bool enable)
{
	_initialized = false;
	_targetPos = 0;
	_currentPos = 0;
	setExternalRange(0,180);
	setInternalRange(0,180);
	_pin = pin;	
}

MFServo::MFServo() : _servo() {}

MFServo::MFServo(uint8_t pin, bool enable) : _servo()
{				
	attach(pin, enable);
}

void MFServo::setExternalRange(int min, int max)
{
	_mapRange[0] = min;
	_mapRange[1] = max;
}

void MFServo::setInternalRange(int min, int max)
{
	_mapRange[2] = min;
	_mapRange[3] = max;
}
