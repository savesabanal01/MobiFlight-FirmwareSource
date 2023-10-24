//
// Servos.h
//
// (C) MobiFlight Project 2022
//

#pragma once

namespace Servos
{
    uint8_t Add(uint8_t pin);
    void    Clear();
    void    OnSet();
    int16_t getActualValue(uint8_t channel);
    void    update();
}

// Servos.h