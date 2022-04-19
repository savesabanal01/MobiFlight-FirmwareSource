//
// Button.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <stdint.h>

namespace Button
{
void Add(uint8_t pin, char const *name);
void Clear();
void readPin();
void readPin();
void OnTrigger();
}

// Button.h