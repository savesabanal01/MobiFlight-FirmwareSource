#pragma once

#include <Arduino_GFX_Library.h>

#define TFT_WIDTH           480
#define TFT_HEIGHT          480
#define MAX_CLIPPING_RADIUS 300

extern Arduino_GFX *gfx;

template <typename T>
static inline void
swap_coord(T &a, T &b)
{
    T t = a;
    a   = b;
    b   = t;
}

namespace TFT
{
    void init();
    void setClippingArea(int16_t ClippingX0, int16_t ClippingY0, int16_t ClippingXwidth, int16_t ClippingYwidth, int16_t clippingRadiusOuter, int16_t clippingRadiusInner);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, bool sel);
    void fillHalfCircleTFT(int16_t x0, int16_t y0, int16_t r, uint16_t colorUpper, uint16_t colorLower);
    void drawPixel(int16_t x, int16_t y, uint16_t color, bool sel);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color, bool sel);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color, bool sel);
}
