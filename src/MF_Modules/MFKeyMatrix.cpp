/*
 * MFMCP23017.h
 *
 * Created: 14.04.2021
 * Author: Ralf Kull
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

MFKeyMatrix::MFKeyMatrix (uint8_t address, const char * name) {
    _adress = address + 0x20;
    _name = name;
    _initialized = false;
}

void MFKeyMatrix::init(void) {
    if (_initialized) return;
    _initialized = true;
    Wire.begin();
    Wire.setClock(400000);
    _mcp.init(_adress);
    _mcp.portMode(MCP23017Port::A, 0x00);                   //Port A as output
    _mcp.portMode(MCP23017Port::B, 0xFF);                   //Port B as input
    _mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);     //Reset port A 
    _mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);     //Reset port B
    _mcp.writePort(MCP23017Port::A, 0xFF);                  //Set all outputs from Port A to 1
}

void MFKeyMatrix::update(void) {
    uint8_t actual_status = 0;
    uint8_t column4bit = 0;
    for (uint8_t column=0; column<8; column++) {
        _mcp.writePort(MCP23017Port::A, ~(1<<column));
        actual_status = _mcp.readPort(MCP23017Port::B);
        if (actual_status != old_status[column]) {
            for (uint8_t bit = 0; bit < 7; bit++) {
                if ((actual_status&(1<<bit)) != (old_status[column]&(1<<bit))) {
                    if (!(actual_status&(1<<bit)) && _handlerList[btnOnPress]!= NULL) {
                        (*_handlerList[btnOnPress])(btnOnPress, (column4bit + bit), _name);
                    }
                    if (actual_status&(1<<bit) && _handlerList[btnOnRelease] != NULL) {
                        (*_handlerList[btnOnRelease])(btnOnRelease, (column4bit + bit), _name);
                    }
                }
            }
            old_status[column] = actual_status;
        }
        column4bit += 8;
    }
}


void MFKeyMatrix::detach()
{
  _initialized = false;
}

void MFKeyMatrix::attachHandler(byte eventId, buttonEvent newHandler)
{
  _handlerList[eventId] = newHandler;
}

#endif
