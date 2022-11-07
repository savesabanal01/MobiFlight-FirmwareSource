#include <Arduino.h>
#include "TFT.h"

// Library instance
TFT_eSPI tft = TFT_eSPI();
// Create two sprites for a DMA toggle buffer
TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};
// Pointers to start of Sprites in RAM (these are then "image" pointers)
uint16_t *sprPtr[2];

namespace TFT
{
    void init()
    {
        // #########################################################################
        //  reduce systemfrequency to 125MHz to get max SPI speed for 62.5 MHz
        //  which is the maximum for most displays.
        //  The SPI clock rate can only be set to an integer division of the processor clock.
        //  The library will drop the clock to the next lower nearest SPI frequency.
        //  So, when the processor frequency is increased to say 133MHz,
        //  then the maximum SPI rate is now 66500000. This means if you specify 62500000
        //  as the frequency then the library will drop the rate to the next lowest value of 33250000,
        //  so you will see a speed drop.
        //  REMERK!! Changing to 125MHz or using PIO for SPI disables servo functionality
        //  as the servo implementation uses also the PIO
        //  if servo is not needed, uncomment the following function
        //  this will speed up SPI transfer
        //  For this example 24.7fps will be increased to 40fps
        //  Using PIO for SPI will additionally free up the MISO pin
        //  running on 133MHz gives 26.2fps, running on 125MHz gives 40.3fps
        // #########################################################################
        // set_sys_clock_khz(125000, false);

        // Changing SPI frequency to clk_sys/2
        // clk_peri does not have a divider, so in and out frequencies must be the same
        clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, clock_get_hz(clk_sys), clock_get_hz(clk_sys));

        tft.init();
        tft.initDMA();
        tft.fillScreen(TFT_BLACK);
        tft.setRotation(0);
        randomSeed(analogRead(A0));
    }
    int32_t *checkClippingArea;

    int32_t clippingCenterX;
    int32_t clippingCenterY;
    int32_t clippingWidthX;
    int32_t clippingWidthY;
    int32_t clippingRadius;

    // setup clipping area
    void setClippingArea(int32_t ClippingX0, int32_t ClippingY0, int32_t ClippingXwidth, int32_t ClippingYwidth, int32_t ClippingRadius)
    {
        *checkClippingArea = new int32_t[ClippingRadius];
        checkClippingArea[0] = ClippingRadius;
        for (uint8_t i = 1; i < ClippingRadius; i++) {
            checkClippingArea[i] = sqrt(ClippingRadius * ClippingRadius - i * i);
        }
        clippingCenterX = ClippingX0;
        clippingCenterY = ClippingY0;
        clippingWidthX  = ClippingXwidth;
        clippingWidthY  = ClippingYwidth;
        clippingRadius  = ClippingRadius;
    }

    // #########################################################################
    // Helper functions transferred from the lib for a round clipping area
    // Before using this functions refresh rate was 21ms
    // Without Clipping it mostly 21ms, sometimes 70ms
    // With rectangular clipping it is still 21ms with sometimes 70ms
    // With round clipping it is 21 - 22ms
    // #########################################################################

    /***************************************************************************************
    ** Function name:           drawLine
    ** Description:             draw a line between 2 arbitrary points
    ***************************************************************************************/
    // Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
    // an efficient FastH/V Line draw routine for line segments of 2 pixels or more
    void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool sel)
    {
        bool steep = abs(y1 - y0) > abs(x1 - x0);
        if (steep) {
            swap_coord(x0, y0);
            swap_coord(x1, y1);
        }

        if (x0 > x1) {
            swap_coord(x0, x1);
            swap_coord(y0, y1);
        }

        int32_t dx = x1 - x0, dy = abs(y1 - y0);

        int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

        if (y0 < y1) ystep = 1;

        // Split into steep and not steep for FastH/V separation
        if (steep) {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawPixel(y0, xs, color, sel);
                    else
                        drawFastVLine(y0, xs, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
                    err += dx;
                }
            }
            if (dlen) drawFastVLine(y0, xs, dlen, color, sel);
        } else {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawPixel(xs, y0, color, sel);
                    else
                        drawFastHLine(xs, y0, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
                    err += dx;
                }
            }
            if (dlen) drawFastHLine(xs, y0, dlen, color, sel);
        }
    }

    /***************************************************************************************
    ** Function name:           drawFastHLine
    ** Description:             draw a horizontal line
    ***************************************************************************************/
    void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color, bool sel)
    {
        // First check upper and lower limits, it's quite easy
        if (clippingRadius > 0) {
            if (y <= clippingCenterY - clippingRadius || y >= clippingCenterY + clippingRadius) return;
        } else {
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
        }
        // always draw from left to right
        if (w < 0) {
            x -= w;
            w *= -1;
        }
        int32_t xE = x + w;
        // check if pixel is inside the radius
        if (clippingRadius > 0) {
            // check left and right limit and set start / end point accordingly from look up table for the given x position
            if (x <= clippingCenterX - checkClippingArea[abs(y - clippingCenterY)]) x = clippingCenterX - checkClippingArea[abs(y - clippingCenterY)];
            if (xE >= clippingCenterX + checkClippingArea[abs(y - clippingCenterY)]) xE = clippingCenterX + checkClippingArea[abs(y - clippingCenterY)];
        } else {
            // check left and right limit and set start / end point accordingly
            if (x <= clippingCenterX - clippingWidthX / 2) x = clippingCenterX - clippingWidthX / 2 + 1;
            if (xE >= clippingCenterX + clippingWidthX / 2) xE = clippingCenterX + clippingWidthX / 2 - 1;
        }
        spr[sel].drawFastHLine(x, y, xE - x + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawFastVLine
    ** Description:             draw a vertical line
    ***************************************************************************************/
    void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color, bool sel)
    {
        if (clippingRadius > 0) {
            if (x <= clippingCenterX - clippingRadius || x >= clippingCenterX + clippingRadius) return;
        } else {
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
        }
        if (h < 0) {
            y -= h;
            h *= -1;
        }
        int32_t yE = y + h;
        if (clippingRadius > 0) {
            // check upper and lower limit and set start / end point accordingly from look up table for the given x position
            if (y <= clippingCenterY - checkClippingArea[abs(x - clippingCenterX)]) y = clippingCenterY - checkClippingArea[abs(x - clippingCenterX)];
            if (yE >= clippingCenterY + checkClippingArea[abs(x - clippingCenterX)]) yE = clippingCenterY + checkClippingArea[abs(x - clippingCenterX)];
        } else {
            // check left and right limit and set start / end point accordingly
            if (y <= clippingCenterY - clippingWidthY / 2) y = clippingCenterY - clippingWidthY / 2 + 1;
            if (yE >= clippingCenterY + clippingWidthY / 2) yE = clippingCenterY + clippingWidthY / 2 - 1;
        }
        spr[sel].drawFastVLine(x, y, yE - y + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawPixel
    ** Description:             push a single pixel at an arbitrary position
    ***************************************************************************************/
    void drawPixel(int32_t x, int32_t y, uint32_t color, bool sel)
    {
        if (clippingRadius > 0) {
            // First do a rect clipping
            if (x <= clippingCenterX - clippingRadius || x >= clippingCenterX + clippingRadius) return;
            if (y <= clippingCenterY - clippingRadius || y >= clippingCenterY + clippingRadius) return;
            // next check if Pixel is within circel or outside
            if (y < clippingCenterY - checkClippingArea[abs(x - clippingCenterX)]) return;
            if (y > clippingCenterY + checkClippingArea[abs(x - clippingCenterX)]) return;
        } else {
            // for a rect clipping area just check upper/lower and left/right limit
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
        }
        spr[sel].drawPixel(x, y, color);
    }

    /***************************************************************************************
    ** Function name:           fillHalfCircle
    ** Description:             draw a filled circle, upper or lower part
    ***************************************************************************************/
    // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
    // Improved algorithm avoids repetition of lines
    void fillHalfCircleSprite(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower, bool sel)
    {
        int32_t x  = 0;
        int32_t dx = 1;
        int32_t dy = r + r;
        int32_t p  = -(r >> 1);

        spr[sel].drawFastHLine(x0 - r, y0, dy + 1, colorUpper);

        while (x < r) {

            if (p >= 0) {
                spr[sel].drawFastHLine(x0 - x, y0 - r, dx, colorUpper);
                spr[sel].drawFastHLine(x0 - x, y0 + r, dx, colorLower);
                dy -= 2;
                p -= dy;
                r--;
            }

            dx += 2;
            p += dx;
            x++;

            spr[sel].drawFastHLine(x0 - r, y0 - x, dy + 1, colorUpper);
            spr[sel].drawFastHLine(x0 - r, y0 + x, dy + 1, colorLower);
        }
    }

 /***************************************************************************************
    ** Function name:           fillHalfCircle
    ** Description:             draw a filled circle, upper or lower part
    ***************************************************************************************/
    // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
    // Improved algorithm avoids repetition of lines
    void fillHalfCircleTFT(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower)
    {
        int32_t x  = 0;
        int32_t dx = 1;
        int32_t dy = r + r;
        int32_t p  = -(r >> 1);

        tft.drawFastHLine(x0 - r, y0, dy + 1, colorUpper);

        while (x < r) {

            if (p >= 0) {
                tft.drawFastHLine(x0 - x, y0 - r, dx, colorUpper);
                tft.drawFastHLine(x0 - x, y0 + r, dx, colorLower);
                dy -= 2;
                p -= dy;
                r--;
            }

            dx += 2;
            p += dx;
            x++;

            tft.drawFastHLine(x0 - r, y0 - x, dy + 1, colorUpper);
            tft.drawFastHLine(x0 - r, y0 + x, dy + 1, colorLower);
        }
    }
} // end of namespace TFT