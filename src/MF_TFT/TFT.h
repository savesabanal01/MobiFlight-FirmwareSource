#pragma once

#include "TFT.h"
#include <TFT_eSPI.h>
#include <SD.h>

// Define the width and height according to the TFT and the
// available memory. The sprites will require:
//     DWIDTH * DHEIGHT * 2 bytes of RAM
// Note: for a 240 * 320 area this is 150 Kbytes!
#define DWIDTH  240
#define DHEIGHT 320

extern TFT_eSPI tft;
extern TFT_eSprite spr[];
extern uint16_t *sprPtr[];

void tft_init();
