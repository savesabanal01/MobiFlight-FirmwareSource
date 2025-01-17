//
// MFServo.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <ESP32Servo.h>
#else
#include <Servo.h>
#endif

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
    long    _targetPos;
    long    _currentPos;
    int     speed;
};

// MFServo.h