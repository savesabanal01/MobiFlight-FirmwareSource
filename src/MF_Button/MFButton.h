//
// MFButton.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>

extern "C" {
// callback functions always follow the signature: void cmd(void);
typedef void (*buttonEvent)(byte, uint8_t, const char *);
};

enum {
    btnOnPress,
    btnOnRelease,
};

/////////////////////////////////////////////////////////////////////
/// \class MFButton MFButton.h <MFButton.h>
class MFButton
{
public:
    MFButton(uint8_t pin = 1, const char * name = "Button", uint8_t bitPosition = 0);
    static void attachHandler(buttonEvent newHandler);    
    void update();
    void readPin();
    void trigger(uint8_t state);
    void triggerOnPress();
    void triggerOnRelease();

private:
    static buttonEvent   _handler;    
    bool          _state;
    const char *  _name;
    uint8_t       _pin;
    uint8_t _arrayPosition;
};

// MFButton.h