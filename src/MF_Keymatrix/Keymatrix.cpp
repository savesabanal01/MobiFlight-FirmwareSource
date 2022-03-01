#include <Arduino.h>
#include "MFKeymatrix.h"
#include "allocateMem.h"
#include "mobiflight.h"
#include "commandmessenger.h"
#include "Keymatrix.h"
#include "MFBoards.h"

namespace Keymatrix
{
MFKeymatrix keymatrix[MAX_KEYMATRIX];
uint8_t keymatrixRegistered = 0;

/*      it's done via Button
void handlerKeyMatrixOnChange(uint8_t eventId, uint8_t pin, const char *name)
{
//  cmdMessenger.sendCmdStart(kKeyMatrixChange);
  cmdMessenger.sendCmdStart(kButtonChange);
  cmdMessenger.sendCmdArg(name);
  cmdMessenger.sendArg("-");                          // send name and pin w/ delimiter "-"
  cmdMessenger.sendArg(pin + KEYMATRIX_BASE_BUTTON);  // send name and pin w/ delimiter "-"
//  cmdMessenger.sendCmdArg(pin);                     // send name and pin w/ delimiter "," -> uncomment both above (sendArg())
  cmdMessenger.sendCmdArg(eventId);
  cmdMessenger.sendCmdEnd();
};
*/

void Add(uint8_t adress, char const * name = "Keymatrix") {
  if (keymatrixRegistered == MAX_KEYMATRIX)
    return;
  keymatrix[keymatrixRegistered] = MFKeymatrix(adress, name);
  keymatrix[keymatrixRegistered].init();
//  keymatrix[keymatrixRegistered].attachHandler(handlerKeyMatrixOnChange);
  keymatrixRegistered++;
}

void Clear() {
  for(int i=0; i!=keymatrixRegistered; i++) {
    keymatrix[i].detach();
  }
  keymatrixRegistered = 0;
}

void read() {
  for(int i=0; i!=keymatrixRegistered; i++) {
    keymatrix[i].update();
  }
}

}
