/*
 * MFMCP23017.h
 *
 * Created: 17.11.2021
 * Author: Ralf Kull
 * 
 * see also: https://ww1.microchip.com/downloads/en/AppNotes/01081a.pdf
 * for using interrupt functionality to get changed button
 * 
 */

#include "Arduino.h"
#include "MFBoards.h"

#if MF_KEYMATRIX_SUPPORT == 1
#include "MFKeyMatrix.h"

enum
{
  btnOnPress,
  btnOnRelease,
};

MFKeymatrix::MFKeymatrix (uint8_t address, const char * name) {
    _adress = address;
    _name = name;
    _initialized = false;
}

void MFKeymatrix::init(void) {
    if (_initialized) return;
    _initialized = true;
    Wire.begin();
    Wire.setClock(400000);
    _mcp.init(_adress);
    _mcp.portMode(MCP23017Port::A, 0x00);                           // Port A as output, columns
    _mcp.portMode(MCP23017Port::B, 0xFF, 0xFF);                     // Port B as input w/ PullUps, rows
    _mcp.interruptMode(MCP23017InterruptMode::Or);                  // Interrupt on one line, not really needed here as only Port B is input
    _mcp.interrupt(MCP23017Port::B, CHANGE);                        // interrupt on changing
    _mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);             // Reset port A
    _mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);             // Reset port B
    _mcp.clearInterrupts();                                         // And clear all interrupts which could come from initialization
    _mcp.writePort(MCP23017Port::A, 0x00);                          // Set all outputs from Port A to 0 to get an interrupt on keypress
}

void MFKeymatrix::update(void) {
    uint8_t actual_status = 0;                                      // reflects the actual status after read the row
    uint8_t column4bit = 0;                                         // for calculation of button number, avoids multiplication of column by 8
    uint8_t portB = 0;                                              // bit position is changed button

    portB= _mcp.readRegister(MCP23017Register::INTF_B);             // read interrupt register from portB to check if an interrupt has occur and from which row
    if (!portB) return;                                             // if no interrupt occur do not check Keymatrix for changed button

    _mcp.writePort(MCP23017Port::A, 0xFF);                          // Set all outputs from Port A to 1,
    for (uint8_t column=0; column<8; column++) {                    // Scan each column for pressed/released key
        _mcp.writePort(MCP23017Port::A, ~(1<<column));              // set columns separate to LOW to find changed button
        actual_status = _mcp.readPort(MCP23017Port::B);             // and read the row
        if ((actual_status&portB) != (old_status[column]&portB)) {  // check for correct column, row is already known from interrupt register
/* *********************************************************************************************
    At this point the changed button is determinied
    button = column4bit + bitLocation
    result could be stored as virtuell button to be handled in button routine
    in this case next both if() are not required here, would be done in button routine
    must be substituted by un-/setting the virtual button
********************************************************************************************** */
            if (!(actual_status&portB) && _handlerList[btnOnPress]!= NULL) {    // check for pressed or released
                (*_handlerList[btnOnPress])(btnOnPress, (column4bit + getBitLocation(portB)), _name);   // and send event
            }
            if (actual_status&portB && _handlerList[btnOnRelease] != NULL) {
                (*_handlerList[btnOnRelease])(btnOnRelease, (column4bit + getBitLocation(portB)), _name);
            }
/* ***************************************************************************************** */
            old_status[column] = actual_status;                     // store actual status as old status to detect next button change
        }
        column4bit += 8;                                            // for calculating button number on next column
    }
    _mcp.writePort(MCP23017Port::A, 0x00);                          // Set all columns to 0 to get an interrupt on next button change
    _mcp.clearInterrupts();                                         // and clear the interrupt to capture the next
}


void MFKeymatrix::detach()
{
  _initialized = false;
}

void MFKeymatrix::attachHandler(byte eventId, buttonEvent newHandler)
{
  _handlerList[eventId] = newHandler;
}

/* **************************************************************************************************
    see https://stackoverflow.com/questions/14429661/determine-which-single-bit-in-the-byte-is-set
        http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog

    BUT a double press on same row may not be detected, maybe better a for() loop over the rows...
    OR it is very unlikely that a button change happens exactly the same time
    -> some more tests required
*************************************************************************************************** */
uint8_t MFKeymatrix::getBitLocation(uint8_t c) {
  // c is in {1, 2, 4, 8, 16, 32, 64, 128}, returned values are {0, 1, ..., 7}
  return (((c & 0xAA) != 0) |
          (((c & 0xCC) != 0) << 1) |
          (((c & 0xF0) != 0) << 2));
}

#endif
