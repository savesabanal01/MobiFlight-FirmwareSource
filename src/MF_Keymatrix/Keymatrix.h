//
// KeyMatrix.h
//
// (C) MobiFlight Project 2022
//

#pragma once

namespace Keymatrix
{
void Add(uint8_t columnCount, uint8_t columnPins[], uint8_t rowCount, uint8_t rowPins[], const char *name = "KeyMatrix");
void Clear();
void read();
}