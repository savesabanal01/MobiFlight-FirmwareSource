#pragma once

#include <TFT_eSPI.h>
#include <SD.h>


extern TFT_eSPI tft;
extern TFT_eSprite spr[];
extern uint16_t *sprPtr[];
extern int32_t  startMillis;

void tft_init();
