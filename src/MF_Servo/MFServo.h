//
// MFServo.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#include <Servo.h>

class MFServo
{
public:
    MFServo();
    void attach(uint8_t pin, bool enable);
    void detach();
    void setExternalRange(int min, int max);
    void setInternalRange(int min, int max);
    void moveTo(int absolute);
    void update();

private:
    uint8_t _pin;
    int     _mapRange[4];
    bool    _initialized;
    Servo   _servo;
    uint8_t _targetPos;
    uint8_t _currentPos;    
    uint8_t _step;
    uint8_t _max_step;
    uint8_t _max_step_limit;
    uint32_t _millis_start;
};

// MFServo.h