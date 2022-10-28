#pragma once

#include <TFT_eSPI.h>
#include <SD.h>

extern TFT_eSPI    tft;
extern TFT_eSprite spr[];
extern uint16_t   *sprPtr[];

namespace TFT {
void init();
void setClippingArea(int32_t ClippingX0, int32_t ClippingY0, int32_t ClippingXwidth, int32_t ClippingYwidth, int32_t ClippingRadius,  int8_t clippingSize);
void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool sel);
void fillHalfCircle(int32_t x0, int32_t y0, int32_t r, uint32_t color, bool upper, bool sel);
void drawPixel(int32_t x, int32_t y, uint32_t color, bool sel);
void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color, bool sel);
void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color, bool sel);
}
