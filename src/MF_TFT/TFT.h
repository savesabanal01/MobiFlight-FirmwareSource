#pragma once

#include <TFT_eSPI.h>
#include <SD.h>


extern TFT_eSPI tft;
extern TFT_eSprite spr[];
extern uint16_t *sprPtr[];

void tft_init();
