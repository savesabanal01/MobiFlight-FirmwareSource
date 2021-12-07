// MFServo.cpp
//
// Copyright (C) 2013-2014

#include "MFServo.h"

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
			_max_step = abs(_currentPos - _targetPos)/4;
			if (_max_step > 7) _max_step = 7;
			_max_step_limit = ((_max_step * _max_step) + _max_step) / 2;	// gaussian sum formula, calculates the steps if decreased by one each update()
																			// if acceleration should be used, the required numbers of steps have to be calculated and added
			_max_step_limit++;												// for safety be at step=1 one position step before target position
			_step = _max_step;												// start with max. calculated steps
			if (!_step) _step = 1;											// at least stepwidth of one is required
    }
}

void MFServo::update() {
	// after reaching final position
	// detach the servo to prevent continuous noise
    if (_currentPos == _targetPos) { 
		// detach(); 
		return; 
	}

	int16_t delta = _currentPos - _targetPos;
	if (abs(delta) < _max_step_limit) {						// if delta position is less than the steps which are required to slow down
		if (_step > 1) _step--;								// reduce speed by decreasing the steps by one
	}

    if (delta > 0) {
		_currentPos-= _step;
	} else {
		_currentPos+= _step;
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
