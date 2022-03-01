/*
 * MFKeyMatrix.h
 *
 * Created: 17.11.2021
 * Author: Ralf Kull
 * version 1.0 Initial release
 * Copyright (C) 2021
 * 
 * see also: https://ww1.microchip.com/downloads/en/AppNotes/01081a.pdf
 * for using interrupt functionality to get changed button
 * 
 */

#pragma once

#include <Arduino.h>
#include <MFBoards.h>
#include "MCP23017.h"

//#define KEYMATRIX_BASE_BUTTON   100     // first button from matrix '0' will be reported as 100

extern "C"
{
  // callback functions always follow the signature: void cmd(void);
  typedef void (*keymatrixEvent) (byte, uint8_t, const char *);
};

class MFKeymatrix
{
private:
    bool                  _initialized = false;
    uint8_t               _adress;
    uint8_t               old_status[8] = {0x00};
    MCP23017              _mcp;
    static keymatrixEvent _handler;
    const char *          _name;
    uint8_t               getBitLocation(uint8_t c);
    uint8_t               _calculate = false;

public:
    MFKeymatrix(uint8_t adress = 0x20, const char * name = "Button");
    void          init(void);
    void          update(void);
    void          detach(void);
    static void   attachHandler(keymatrixEvent newHandler);
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

Column is MCP23017 Port A, must be set to output, all LOW to get changed button
Row is MCP23017 Port B, must be set to input with pullup
*******************************************************************************************/
