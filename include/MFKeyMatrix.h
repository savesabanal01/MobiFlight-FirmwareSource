/// \version 1.0 Initial release
/// \author  elral66 DO NOT CONTACT THE AUTHOR DIRECTLY: USE THE LISTS
// Copyright (C) 2013-2021

#ifndef MFKEYMATRIX_h
#define MFKEYMATRIX_h

#include <Arduino.h>
#include <MFBoards.h>

#include "MCP23017.h"
#define REPEAT_START_TIME   50
#define REPEAT_TIME         15


extern "C"
{
  // callback functions always follow the signature: void cmd(void);
  typedef void (*buttonEvent) (byte, uint8_t, const char *);
};


class MFKeyMatrix
{
private:
    bool                _initialized = false;
    uint8_t             _adress;
    uint8_t             old_status[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    MCP23017            _mcp;
    buttonEvent         _handlerList[2];
    const char *        _name;

public:
    // Constructor
    MFKeyMatrix(uint8_t adress = 0, const char * name = "Button");        // address = MCP23017 adress for Keymatrix 8x8
    void          init(void);
    // call this function every some milliseconds or by using an interrupt for handling state changes of the rotary encoder.
//    void          tick(void);
    void          update(void);
    void          detach(void);
    void          attachHandler(byte eventId, buttonEvent newHandler);
};


#endif


/*******************************************************************************************
        Column  0   1   2   3   4   5   6   7   -> will be set column by column to 0 (Port A)
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

Column is MCP23017 Port A, must be set to output, all HIGH
Row is MCP23017 Port B, must be set to input with pullup
*******************************************************************************************/
