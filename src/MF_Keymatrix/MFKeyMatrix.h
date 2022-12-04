//
// MFKeyMatrix.h
//
// (C) MobiFlight Project 2022
//

#pragma once

#include <Arduino.h>
#include <MFBoards.h>

extern "C" {
// callback functions always follow the signature: void cmd(void);
typedef void (*keymatrixEvent)(byte, uint8_t, const char *);
};

class MFKeymatrix
{
private:
    bool                  _initialized = false;
    uint8_t               _columnCount;
    uint8_t              *_columnPins;
    uint8_t               _rowCount;
    uint8_t              *_rowPins;
    uint8_t               _rowStatusAllColumn;
    uint8_t               old_status[MAX_COLUMN_KEYMATRIX] = {0x00};
    static keymatrixEvent _handler;
    const char           *_name;
    void                  triggerOnPress(uint8_t pin);
    void                  triggerOnRelease(uint8_t pin);

public:
    MFKeymatrix(uint8_t columnCount, uint8_t columnPins[], uint8_t rowCount, uint8_t rowPins[], const char *name = "KeyMatrix");
    void        init(void);
    void        update(void);
    void        trigger(uint8_t state, uint8_t pin);
    void        detach(void);
    static void attachHandler(keymatrixEvent newHandler);
};

/*******************************************************************************************
        Column  0   1   2   3   4   5   6   7   -> will be set column by column to output and LOW (Port A)
                |   |   |   |   |   |   |   |
        Row 0 <-0---8---16--24--32--40--48--56
        Row 1 <-1---9---17--X---X---X---X---57
        Row 2 <-2---10--X---X---X---X---X---58
        Row 3 <-3---X---X---X---X---X---X---59
        Row 4 <-4---X---X---X---X---X---X---60
        Row 5 <-5---X---X---X---X---X---X---61
        Row 6 <-6---X---X---X---X---X---X---62
        Row 7 <-7---15--23--31--39--47--55--63
         ^
        PortB

Column must be set to output, all LOW to get changed button
Row must be set to input with pullup
*******************************************************************************************/
