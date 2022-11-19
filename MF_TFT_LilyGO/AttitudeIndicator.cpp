
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16
// Adapted to use sprites and DMA transfer for RP2040
// additionally rect and round instrument implemented

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

//  The sprites will require:
//     SPRITE_WIDTH * SPRITE_HEIGTH * 2 bytes of RAM
// Note: for a 240 * 320 area this is 150 Kbytes!

// Hmhm, but an outer rand should not get changed
// so dimension of 200 x 200 should be OK and
// no rect clipping would be required...
// It's also valid for the round clipping area
// sprite must be max. 2 * Radius in x and y direction...
// But adapt Sprite center position...
#define SPRITE_WIDTH_RECT            200
#define SPRITE_HEIGTH_RECT           280                    // with this dimensions 112.5kBytes are required
#define SPRITE_X0_RECT               20                     // upper left x position where to plot
#define SPRITE_Y0_RECT               20                     // upper left y position where to plot
#define INSTRUMENT_CENTER_X0_RECT    SPRITE_WIDTH_RECT / 2  // x mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_CENTER_Y0_RECT    SPRITE_HEIGTH_RECT / 2 // y mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_OUTER_WIDTH_RECT  240                    // width of outer part of instrument
#define INSTRUMENT_OUTER_HEIGHT_RECT 280                    // height of outer part of instrument
#define CLIPPING_XWIDTH              240                    // width of clipping area for rect instrument around INSTRUMENT_CENTER_X0_RECT, if higher than Sprite dimension not considered
#define CLIPPING_YWIDTH              320                    // height of clipping area for rect instrument around INSTRUMENT_CENTER_Y0_RECT, if higher than Sprite dimension not considered
//#define SPRITE_DIM_RADIUS            120                    // dimension for x and y direction of sprite, including outer part
//#define SPRITE_X0_ROUND              0                      // upper left x position where to plot
//#define SPRITE_Y0_ROUND              40                     // upper left y position where to plot
#define INSTRUMENT_CENTER_X0_ROUND 240                          // x mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_CENTER_Y0_ROUND 240                  // y mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_OUTER_RADIUS    120                  // radius of outer part of instrument
#define INSTRUMENT_MOVING_RADIUS   99  // radius of moving part of instrument
//#define CLIPPING_RADIUS              220                    // radius of clipping area for round instrument including the outer part!
#define HOR        400    // Horizon vector line, length must be at least sqrt(SPRITE_WIDTH_RECT^2 + SPRITE_HEIGTH_RECT^2) = 344
#define MAX_PITCH  100    // Maximum pitch shouls be in range +/- 80 with HOR = 172, 20 steps = 10 degrees on drawn scale
#define BROWN      0xFD20 // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE   0x02B5 // 0x0318 //0x039B //0x34BF
#define DARK_RED   RED    // 0x8000
#define DARK_GREY  BLACK  // ILI9341_DARKGREY
#define LIGHT_GREY BLACK  // ILI9341_LIGHTGREY

#define DEG2RAD 0.0174532925

int     roll           = 0;
int     pitch          = 0;
int     last_roll      = 0;
int     last_pitch     = 0;
uint8_t instrumentType = 0; // 1 = round instrument, 2 = rect instrument

void drawCentreString(const char *buf, int x, int y, uint8_t size)
{
    int16_t  x1, y1;
    uint16_t w, h;
    gfx->setTextSize(size);
    gfx->getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
    gfx->setCursor(x - w / 2, y - h / 2);
    gfx->print(buf);
}

namespace AttitudeIndicator
{
    void updateHorizon(int roll, int pitch);
    void drawHorizon(int roll, int pitch, bool sel);
    void drawScale(bool sel);
    void drawOuter();

    // #########################################################################
    // Setup, runs once on boot up
    // #########################################################################
    void init(uint8_t type)
    {
        instrumentType = type;
        pitch          = 0;
        roll           = 0;
        last_pitch     = 0;
        last_roll      = 0;
        gfx->fillScreen(BLACK);
        gfx->startWrite(); // TFT chip select held low permanently

        // draw outer part of instrument
        drawOuter();

        // Draw fixed text at top/bottom of screen
        gfx->setTextColor(WHITE);
        // gfx->setTextDatum(TC_DATUM); // Centre middle justified
        drawCentreString("Demo Attitude Indicator", TFT_WIDTH / 2, 1, 1);
        drawCentreString("Based on Bodmer's example", TFT_WIDTH / 2, 10, 1);
        gfx->setTextColor(YELLOW);
    }

    void stop()
    {
        gfx->endWrite();
    }

    // #########################################################################
    // Main loop, keeps looping around
    // #########################################################################

    void loop()
    {
        // Roll is in degrees in range +/-180
        // roll = random(361) - 180;
        roll++;
        if (roll == 180) roll = -180;

        // Pitch is in y coord (pixel) steps, 20 steps = 10 degrees on drawn scale
        // Maximum pitch shouls be in range +/- 80 with HOR = 172
        // pitch = 10; //random(2 * INSTRUMENT_CENTER_Y0_RECT) - INSTRUMENT_CENTER_Y0_RECT;
        pitch++;

        // check for overflow, if so then move step by step
        // to lower pitch
        if (pitch > MAX_PITCH) {
            while (pitch > -MAX_PITCH + 3) {
                updateHorizon(roll, pitch);
                pitch -= 3;
            };
        }
        // to upper pitch
        if (pitch < -MAX_PITCH) {
            while (pitch < MAX_PITCH - 3) {
                updateHorizon(roll, pitch);
                pitch += 3;
            };
        }
        updateHorizon(roll, pitch);
    }

    // #########################################################################
    // Update the horizon with a new roll (angle in range -180 to +180)
    // #########################################################################
    void updateHorizon(int roll, int pitch)
    {
        bool draw        = 1;
        int  delta_pitch = 0;
        int  pitch_error = 0;
        int  delta_roll  = 0;

        if (last_roll == -180 && roll > 0) {
            last_roll = 180;
        }
        if (last_roll == 180 && roll < 0) {
            last_roll = -180;
        }

        while ((last_pitch != pitch) || (last_roll != roll)) {
            delta_pitch = 0;
            delta_roll  = 0;

            if (last_pitch < pitch) {
                delta_pitch = 1;
                pitch_error = pitch - last_pitch;
            }

            if (last_pitch > pitch) {
                delta_pitch = -1;
                pitch_error = last_pitch - pitch;
            }

            if (last_roll < roll) delta_roll = 1;
            if (last_roll > roll) delta_roll = -1;

            if (delta_roll == 0) {
                if (pitch_error > 1) delta_pitch *= 2;
            }

            drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch, 0);

            last_roll  = roll;
            last_pitch = pitch;
        }
    }

    // #########################################################################
    // Draw the horizon with a new roll (angle in range -180 to +180)
    // #########################################################################
    void drawHorizon(int roll, int pitch, bool sel)
    {
        // Calculate coordinates for line start
        int16_t x0      = (float)cos(roll * DEG2RAD) * HOR;
        int16_t y0      = (float)sin(roll * DEG2RAD) * HOR;
        int16_t x0outer = (float)cos(roll * DEG2RAD) * INSTRUMENT_OUTER_RADIUS;
        int16_t y0outer = (float)sin(roll * DEG2RAD) * INSTRUMENT_OUTER_RADIUS;

        // check in which direction to move
        int16_t xd  = 0;
        int16_t yd  = 1;
        int16_t xdn = 0;
        int16_t ydn = 0;
        // position to draw lines
        int16_t posX, posY, widthX, widthY;

        if (roll > 45 && roll < 135) {
            xd = -1;
            yd = 0;
        }
        if (roll >= 135) {
            xd = 0;
            yd = -1;
        }
        if (roll < -45 && roll > -135) {
            xd = 1;
            yd = 0;
        }
        if (roll <= -135) {
            xd = 0;
            yd = -1;
        }

        if (instrumentType == ROUND_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                // draw outer part
                // Hmmmhmmm, have to re-think how to do this... Seems that an additional clipping radius for the inner circle is required...
                /*
                                TFT::setClippingArea(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, 0, 0, INSTRUMENT_OUTER_RADIUS, INSTRUMENT_MOVING_RADIUS);
                                for (uint8_t i = 6; i > 0; i--) {
                                    xdn    = i * xd;
                                    ydn    = i * yd;
                                    posX   = INSTRUMENT_CENTER_X0_ROUND - x0outer - xdn;
                                    posY   = INSTRUMENT_CENTER_Y0_ROUND - y0outer - ydn;
                                    widthX = INSTRUMENT_CENTER_X0_ROUND + x0outer - xdn;
                                    widthY = INSTRUMENT_CENTER_Y0_ROUND + y0outer - ydn;

                                    TFT::drawLine(posX, posY, widthX, widthY, SKY_BLUE, sel);
                                    posX   = INSTRUMENT_CENTER_X0_ROUND - x0outer + xdn;
                                    posY   = INSTRUMENT_CENTER_Y0_ROUND - y0outer + ydn;
                                    widthX = INSTRUMENT_CENTER_X0_ROUND + x0outer + xdn;
                                    widthY = INSTRUMENT_CENTER_Y0_ROUND + y0outer + ydn;
                                    TFT::drawLine(posX, posY, widthX, widthY, BROWN, sel);
                                }
                                posX   = INSTRUMENT_CENTER_X0_ROUND - x0outer;
                                posY   = INSTRUMENT_CENTER_Y0_ROUND - y0outer;
                                widthX = INSTRUMENT_CENTER_X0_ROUND + x0outer;
                                widthY = INSTRUMENT_CENTER_Y0_ROUND + y0outer;
                                TFT::drawLine(posX, posY, widthX, widthY, WHITE, sel);
                */

                // draw inner moving part
                TFT::setClippingArea(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, 0, 0, INSTRUMENT_MOVING_RADIUS, 0);
                for (uint8_t i = 6; i > 0; i--) {
                    xdn    = i * xd;
                    ydn    = i * yd;
                    posX   = INSTRUMENT_CENTER_X0_ROUND - x0 - xdn;
                    posY   = INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch;
                    widthX = INSTRUMENT_CENTER_X0_ROUND + x0 - xdn;
                    widthY = INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch;
                    TFT::drawLine(posX, posY, widthX, widthY, SKY_BLUE, sel);
                    posX   = INSTRUMENT_CENTER_X0_ROUND - x0 + xdn;
                    posY   = INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch;
                    widthX = INSTRUMENT_CENTER_X0_ROUND + x0 + xdn;
                    widthY = INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch;
                    TFT::drawLine(posX, posY, widthX, widthY, BROWN, sel);
                }

                posX   = INSTRUMENT_CENTER_X0_ROUND - x0;
                posY   = INSTRUMENT_CENTER_Y0_ROUND - y0 - pitch;
                widthX = INSTRUMENT_CENTER_X0_ROUND + x0;
                widthY = INSTRUMENT_CENTER_Y0_ROUND + y0 - pitch;
                TFT::drawLine(posX, posY, widthX, widthY, WHITE, sel);

                gfx->drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS - 0, LIGHT_GREY);
                gfx->drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS - 1, LIGHT_GREY);
                gfx->drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, LIGHT_GREY);

                drawScale(sel);
            }
        }
        if (instrumentType == RECT_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                // draw outer part

                gfx->fillRect(0, 20, 18, 140 - pitch - 1, SKY_BLUE);
                gfx->fillRect(0, 160 - pitch, 18, 140 + pitch, BROWN);

                gfx->fillRect(222, 20, 18, 140 - pitch - 1, SKY_BLUE);
                gfx->fillRect(222, 160 - pitch, 18, 140 + pitch, BROWN);

                // draw inner moving part
                TFT::setClippingArea(INSTRUMENT_CENTER_X0_RECT, INSTRUMENT_CENTER_Y0_RECT, CLIPPING_XWIDTH, CLIPPING_YWIDTH, 0, 0), 0;
                for (uint8_t i = 6; i > 0; i--) {
                    xdn    = i * xd;
                    ydn    = i * yd;
                    posX   = INSTRUMENT_CENTER_X0_RECT - x0 - xdn;
                    posY   = INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch;
                    widthX = INSTRUMENT_CENTER_X0_RECT + x0 - xdn;
                    widthY = INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch;
                    TFT::drawLine(posX, posY, widthX, widthY, SKY_BLUE, sel);
                    posX   = INSTRUMENT_CENTER_X0_RECT - x0 + xdn;
                    posY   = INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch;
                    widthX = INSTRUMENT_CENTER_X0_RECT + x0 + xdn;
                    widthY = INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch;
                    TFT::drawLine(posX, posY, widthX, widthY, BROWN, sel);
                }
                /*
                posX   = INSTRUMENT_CENTER_X0_RECT - x0;
                posY   = INSTRUMENT_CENTER_Y0_RECT - y0 - pitch;
                widthX = INSTRUMENT_CENTER_X0_RECT + x0;
                widthY = INSTRUMENT_CENTER_Y0_RECT + y0 - pitch;
                TFT::drawLine(posX, posY, widthX, widthY, WHITE, sel);
                */

                drawScale(sel);

                gfx->drawRect(SPRITE_X0_RECT - 1, SPRITE_Y0_RECT - 1, SPRITE_WIDTH_RECT + 2, SPRITE_HEIGTH_RECT + 2, DARK_GREY);
                gfx->drawRect(SPRITE_X0_RECT - 1, SPRITE_Y0_RECT - 1, SPRITE_WIDTH_RECT + 2, SPRITE_HEIGTH_RECT + 2, DARK_GREY);
                gfx->drawRect(SPRITE_X0_RECT - 2, SPRITE_Y0_RECT - 2, SPRITE_WIDTH_RECT + 4, SPRITE_HEIGTH_RECT + 4, DARK_GREY);
                gfx->drawRect(SPRITE_X0_RECT - 2, SPRITE_Y0_RECT - 2, SPRITE_WIDTH_RECT + 4, SPRITE_HEIGTH_RECT + 4, DARK_GREY);
            }
        }
    }

    // #########################################################################
    // Draw the information
    // #########################################################################
    void drawScale(bool sel)
    {
        if (instrumentType == ROUND_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            gfx->fillRect(INSTRUMENT_CENTER_X0_ROUND - 1, INSTRUMENT_CENTER_Y0_ROUND - 1, 3, 3, RED);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 30, INSTRUMENT_CENTER_Y0_ROUND, 24, RED);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND + 30 - 24, INSTRUMENT_CENTER_Y0_ROUND, 24, RED);
            gfx->drawFastVLine(INSTRUMENT_CENTER_X0_ROUND - 30 + 24, INSTRUMENT_CENTER_Y0_ROUND, 3, RED);
            gfx->drawFastVLine(INSTRUMENT_CENTER_X0_ROUND + 30 - 24, INSTRUMENT_CENTER_Y0_ROUND, 3, RED);

            // Pitch scale
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND - 40, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND - 30, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND - 20, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND - 10, 12, WHITE);

            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND + 10, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND + 20, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND + 30, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND + 40, 24, WHITE);

            // Pitch scale values
            gfx->setTextColor(WHITE);
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND - 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND - 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND + 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND + 20 - 3);
            gfx->print("10");

            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND - 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND - 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND + 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND + 40 - 3);
            gfx->print("20");

            gfx->setTextColor(WHITE, BLACK); // Text with background
        }
        if (instrumentType == RECT_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            gfx->fillRect(INSTRUMENT_CENTER_X0_RECT - 1, INSTRUMENT_CENTER_Y0_RECT - 1, 3, 3, RED);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 30, INSTRUMENT_CENTER_Y0_RECT, 24, RED);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT + 30 - 24, INSTRUMENT_CENTER_Y0_RECT, 24, RED);
            gfx->drawFastVLine(INSTRUMENT_CENTER_X0_RECT - 30 + 24, INSTRUMENT_CENTER_Y0_RECT, 3, RED);
            gfx->drawFastVLine(INSTRUMENT_CENTER_X0_RECT + 30 - 24, INSTRUMENT_CENTER_Y0_RECT, 3, RED);

            // Pitch scale
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT - 40, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT - 30, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT - 20, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT - 10, 12, WHITE);

            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT + 10, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT + 20, 24, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT + 30, 12, WHITE);
            gfx->drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT + 40, 24, WHITE);

            // Pitch scale values
            gfx->setTextColor(WHITE);
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT - 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT - 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT + 20 - 3);
            gfx->print("10");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT + 20 - 3);
            gfx->print("10");

            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT - 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT - 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT + 40 - 3);
            gfx->print("20");
            gfx->setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT + 40 - 3);
            gfx->print("20");
            gfx->setTextColor(BLACK, BROWN); // Text with background
        }

        // Display justified roll value near bottom of screen
        // gfx->setTextPadding(24);                                             // Padding width to wipe previous number
        char message[40];                                                    // buffer for message
        sprintf(message, " Roll: %4d / Pitch: %3d ", last_roll, last_pitch); // create message
        drawCentreString(message, TFT_WIDTH / 2, TFT_HEIGHT - 9, 1);
    }

    void drawOuter()
    {
        if (instrumentType == ROUND_SHAPE) {
            // fill sprite with not moving area
            TFT::fillHalfCircleSprite(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, SKY_BLUE, BROWN, 0);
            TFT::fillHalfCircleSprite(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, SKY_BLUE, BROWN, 1);
            // and now the "static" area
            TFT::fillHalfCircleTFT(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, SKY_BLUE, BROWN);
            gfx->drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, LIGHT_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            gfx->fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT - SPRITE_HEIGTH_RECT / 2, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, SKY_BLUE);
            gfx->fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, BROWN);
            gfx->fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT - SPRITE_HEIGTH_RECT / 2, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, SKY_BLUE);
            gfx->fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, BROWN);
            // fill area outside sprite with not moving area
            gfx->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT / 2, SKY_BLUE);
            gfx->fillRect(0, 160, TFT_WIDTH, TFT_HEIGHT / 2, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
