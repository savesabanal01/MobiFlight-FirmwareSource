//
// MFKeyMatrix.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFBoards.h"
#include "Arduino.h"
#include "mobiflight.h"
#include "MFKeyMatrix.h"

enum KeyMatrixState {
    btnOnPress,
    btnOnRelease,
};

keymatrixEvent MFKeymatrix::_handler = NULL;

MFKeymatrix::MFKeymatrix(uint8_t columnCount, uint8_t columnPins[], uint8_t rowCount, uint8_t rowPins[], const char *name)
{
    _columnCount  = columnCount;
    _columnPins   = columnPins;
    _rowCount     = rowCount;
    _rowPins      = rowPins;
    _name         = name;
    _rowAllColumn = 0;
    _initialized  = false;
}

void MFKeymatrix::init(void)
{
    if (_initialized) return;

    // set each column to INPUT
    // columns will be set all to OUTPUT to check if a button has changed
    for (uint8_t i = 0; i < _columnCount; i++) {
        pinMode(_columnPins[i], OUTPUT);
        digitalWrite(_columnPins[i], LOW);
        for (uint8_t j = 0; j < _rowCount; j++) {
            old_status[i] |= 1 << j;
        }
    }
    // and set each row to input
    for (uint8_t i = 0; i < _rowCount; i++) {
        pinMode(_rowPins[i], INPUT_PULLUP);
        _rowAllColumn |= (1 << i);
    }
    _initialized = true;
}

void MFKeymatrix::update(void)
{
    if (!_initialized) return;

    uint8_t actual_status = 0; // reflects the actual status after reading the row
    uint8_t column4bit    = 0; // for calculation button number

    // All columns are set to LOW, read in the rows and check against the old status
    // if the actual status is NOT the same as the old one a button press has changed
    // otherwise just return as no button status has changed.
    for (uint8_t i = 0; i < _rowCount; i++) {
        actual_status |= digitalRead(_rowPins[i]) << i;
    }
    // no button status has changed
    if (actual_status == _rowAllColumn)
        return;

    // button status has changed, save the actual one
    _rowAllColumn = actual_status;
    // and prepare for read in the row status for each column
    actual_status = 0;
    // each column will be set to LOW to avoid diodes in keymatrix
    for (uint8_t i = 0; i < _columnCount; i++) {
        pinMode(_columnPins[i], INPUT_PULLUP);
    }

    // set each column one by one to OUTPUT and LOW to check if a button in this row has changed
    for (uint8_t i = 0; i < _columnCount; i++) {
        // set one column to OUTPUT and LOW to check the rows
        pinMode(_columnPins[i], OUTPUT);
        digitalWrite(_columnPins[i], LOW);
        // each row pin will be read in and all will be saved in one byte
        for (uint8_t j = 0; j < _rowCount; j++) {
            actual_status |= digitalRead(_rowPins[j]) << j;
        }
        // check which one input has changed
        if (actual_status != old_status[i]) {
            // and if so check which row has changed
            for (uint8_t j = 0; j < _rowCount; j++) {
                // check bitwise the row
                if ((actual_status & (1 << j)) != (old_status[i] & (1 << j))) {
                    trigger(actual_status & (1 << j), column4bit + j);
                    // set the actual column to INPUT to be prepares for next matrix reading
                    old_status[i] = actual_status;
  //                  return;
                }
            }
            // save the new status
            old_status[i] = actual_status;
        }
        // set the actual column to INPUT to be prepared for next column reading
        pinMode(_columnPins[i], INPUT_PULLUP);
        column4bit += _columnCount; // for calculating button number on next column
        actual_status = 0;
    }

    // and least set all columns back to OUTPUT and LOW
    for (uint8_t i = 0; i < _columnCount; i++) {
        pinMode(_columnPins[i], OUTPUT);
        digitalWrite(_columnPins[i], LOW);
    }
}

void MFKeymatrix::trigger(uint8_t state, uint8_t pin)
{
    (state == LOW) ? triggerOnPress(pin) : triggerOnRelease(pin);
}

void MFKeymatrix::triggerOnPress(uint8_t pin)
{
    if (_handler) {
        (*_handler)(KeyMatrixState::btnOnPress, pin, _name);
    }
}

void MFKeymatrix::triggerOnRelease(uint8_t pin)
{
    if (_handler) {
        (*_handler)(KeyMatrixState::btnOnRelease, pin, _name);
    }
}

void MFKeymatrix::detach()
{
    _initialized = false;
}

void MFKeymatrix::attachHandler(keymatrixEvent newHandler)
{
    _handler = newHandler;
}
