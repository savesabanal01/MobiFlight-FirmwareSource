
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16
// Adapted to use sprites and DMA transfer for RP2040
// additionally rect and round instrument implemented

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

#define TFT_WIDTH  240
#define TFT_HEIGTH 320
// Define the width and height according to the TFT and the
// available memory. The sprites will require:
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
#define SPRITE_DIM_RADIUS            120                    // dimension for x and y direction of sprite, including outer part
#define SPRITE_X0_ROUND              0                      // upper left x position where to plot
#define SPRITE_Y0_ROUND              40                     // upper left y position where to plot
#define INSTRUMENT_CENTER_X0_ROUND   SPRITE_DIM_RADIUS      // x mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_CENTER_Y0_ROUND   SPRITE_DIM_RADIUS      // y mid point in sprite for instrument, complete drawing must be inside sprite
#define INSTRUMENT_OUTER_RADIUS      120                    // radius of outer part of instrument
#define INSTRUMENT_MOVING_RADIUS     100                    // radius of moving part of instrument
#define CLIPPING_RADIUS              120                    // radius of clipping area for round instrument including the outer part!
#define HOR                          350                    // Horizon vector line, length must be at least sqrt(SPRITE_WIDTH_RECT^2 + SPRITE_HEIGTH_RECT^2) = 344
#define BROWN                        0xFD20                 // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE                     0x02B5                 // 0x0318 //0x039B //0x34BF
#define DARK_RED                     0x8000
#define DARK_GREY                    0x39C7
// TFT_TRANSPARENT check how to use
// spr[0].fillSprite(TFT_TRANSPARENT);
// spr[0].setColorDepth(int8_t b);
// spr[0].createSprite(70, 80);
// spr[0].fillSprite(TFT_TRANSPARENT);
// spr[0].pushSprite(x, y, TFT_TRANSPARENT);
// spr[0].getColorDepth(void);
// spr[0].deleteSprite();

#define DEG2RAD 0.0174532925

int     last_roll      = 0;
int     last_pitch     = 0;
uint8_t instrumentType = 0; // 1 = round instrument, 2 = rect instrument
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
        tft.fillScreen(TFT_BLACK);
        // setup clipping area
        if (instrumentType == ROUND_SHAPE)
            TFT::setClippingArea(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, 0, 0, INSTRUMENT_MOVING_RADIUS);
        if (instrumentType == RECT_SHAPE)
            TFT::setClippingArea(INSTRUMENT_CENTER_X0_RECT, INSTRUMENT_CENTER_Y0_RECT, CLIPPING_XWIDTH, CLIPPING_YWIDTH, 0);

        spr[0].setRotation(0);
        spr[1].setRotation(0);
        // spr[0].setColorDepth(8);
        // spr[1].setColorDepth(8);
        if (instrumentType == ROUND_SHAPE) {
            // Create the 2 sprites, each is half the size
            sprPtr[0] = (uint16_t *)spr[0].createSprite(SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS);
            sprPtr[1] = (uint16_t *)spr[1].createSprite(SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS);
            // Move the sprite 1 coordinate datum upwards half the screen height
            // so from coordinate point of view it occupies the bottom of screen
            spr[1].setViewport(0 /* SPRITE_X0_RECT */, -SPRITE_DIM_RADIUS, SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS * 2);
        }
        if (instrumentType == RECT_SHAPE) {
            // Create the 2 sprites, each is half the size of the screen
            sprPtr[0] = (uint16_t *)spr[0].createSprite(SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2);
            sprPtr[1] = (uint16_t *)spr[1].createSprite(SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2);
            // Move the sprite 1 coordinate datum upwards half the screen height
            // so from coordinate point of view it occupies the bottom of screen
            spr[1].setViewport(0 /* SPRITE_X0_RECT */, -SPRITE_HEIGTH_RECT / 2, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT);
        }
        // Define text datum for each Sprite
        spr[0].setTextDatum(MC_DATUM);
        spr[1].setTextDatum(MC_DATUM);

        tft.startWrite(); // TFT chip select held low permanently

        // draw outer part of instrument
        drawOuter();

        // Draw fixed text at top/bottom of screen
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(TC_DATUM); // Centre middle justified
        tft.drawString("Demo Attitude Indicator", TFT_WIDTH / 2, 1, 1);
        tft.drawString("Based on Bodmer's example", TFT_WIDTH / 2, 10, 1);
        tft.setTextColor(TFT_YELLOW);
    }

    void stop()
    {
        tft.endWrite();
        // Delete sprite to free up the RAM
        spr[0].deleteSprite();
        spr[1].deleteSprite();
    }

    // #########################################################################
    // Main loop, keeps looping around
    // #########################################################################
    int roll  = 0;
    int pitch = 0;

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
        if (pitch > 40) pitch = -40;

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
            drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch, 1);
        }
    }

    // #########################################################################
    // Draw the horizon with a new roll (angle in range -180 to +180)
    // #########################################################################
    void drawHorizon(int roll, int pitch, bool sel)
    {
        // Calculate coordinates for line start
        float sx = cos(roll * DEG2RAD);
        float sy = sin(roll * DEG2RAD);

        int16_t x0 = sx * HOR;
        int16_t y0 = sy * HOR;

        int16_t xd  = 0;
        int16_t yd  = 1;
        int16_t xdn = 0;
        int16_t ydn = 0;

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
                xdn = 6 * xd;
                ydn = 6 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch, BROWN, sel);
                xdn = 5 * xd;
                ydn = 5 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch, BROWN, sel);
                xdn = 4 * xd;
                ydn = 4 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch, BROWN, sel);

                xdn = 3 * xd;
                ydn = 3 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch, BROWN, sel);
            }
            xdn = 2 * xd;
            ydn = 2 * yd;
            TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xdn, INSTRUMENT_CENTER_Y0_ROUND + y0 + ydn - pitch, BROWN, sel);

            TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 - xd, INSTRUMENT_CENTER_Y0_ROUND - y0 - yd - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 - xd, INSTRUMENT_CENTER_Y0_ROUND + y0 - yd - pitch, SKY_BLUE, sel);
            TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0 + xd, INSTRUMENT_CENTER_Y0_ROUND - y0 + yd - pitch, INSTRUMENT_CENTER_X0_ROUND + x0 + xd, INSTRUMENT_CENTER_Y0_ROUND + y0 + yd - pitch, BROWN, sel);

            TFT::drawLine(INSTRUMENT_CENTER_X0_ROUND - x0, INSTRUMENT_CENTER_Y0_ROUND - y0 - pitch, INSTRUMENT_CENTER_X0_ROUND + x0, INSTRUMENT_CENTER_Y0_ROUND + y0 - pitch, TFT_WHITE, sel);

            spr[0].drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS, DARK_GREY);
            spr[1].drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS, DARK_GREY);
            spr[0].drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS + 1, DARK_GREY);
            spr[1].drawCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_MOVING_RADIUS + 1, DARK_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                xdn = 6 * xd;
                ydn = 6 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xdn, INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xdn, INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xdn, INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xdn, INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch, BROWN, sel);
                xdn = 5 * xd;
                ydn = 5 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xdn, INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xdn, INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xdn, INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xdn, INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch, BROWN, sel);
                xdn = 4 * xd;
                ydn = 4 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xdn, INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xdn, INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xdn, INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xdn, INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch, BROWN, sel);

                xdn = 3 * xd;
                ydn = 3 * yd;
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xdn, INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xdn, INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch, SKY_BLUE, sel);
                TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xdn, INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xdn, INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch, BROWN, sel);
            }
            xdn = 2 * xd;
            ydn = 2 * yd;
            TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xdn, INSTRUMENT_CENTER_Y0_RECT - y0 - ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xdn, INSTRUMENT_CENTER_Y0_RECT + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xdn, INSTRUMENT_CENTER_Y0_RECT - y0 + ydn - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xdn, INSTRUMENT_CENTER_Y0_RECT + y0 + ydn - pitch, BROWN, sel);

            TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 - xd, INSTRUMENT_CENTER_Y0_RECT - y0 - yd - pitch, INSTRUMENT_CENTER_X0_RECT + x0 - xd, INSTRUMENT_CENTER_Y0_RECT + y0 - yd - pitch, SKY_BLUE, sel);
            TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0 + xd, INSTRUMENT_CENTER_Y0_RECT - y0 + yd - pitch, INSTRUMENT_CENTER_X0_RECT + x0 + xd, INSTRUMENT_CENTER_Y0_RECT + y0 + yd - pitch, BROWN, sel);

            TFT::drawLine(INSTRUMENT_CENTER_X0_RECT - x0, INSTRUMENT_CENTER_Y0_RECT - y0 - pitch, INSTRUMENT_CENTER_X0_RECT + x0, INSTRUMENT_CENTER_Y0_RECT + y0 - pitch, TFT_WHITE, sel);

            tft.drawRect(SPRITE_X0_RECT - 1, SPRITE_Y0_RECT - 1, SPRITE_WIDTH_RECT + 2, SPRITE_HEIGTH_RECT + 2, DARK_GREY);
            tft.drawRect(SPRITE_X0_RECT - 1, SPRITE_Y0_RECT - 1, SPRITE_WIDTH_RECT + 2, SPRITE_HEIGTH_RECT + 2, DARK_GREY);
            tft.drawRect(SPRITE_X0_RECT - 2, SPRITE_Y0_RECT - 2, SPRITE_WIDTH_RECT + 4, SPRITE_HEIGTH_RECT + 4, DARK_GREY);
            tft.drawRect(SPRITE_X0_RECT - 2, SPRITE_Y0_RECT - 2, SPRITE_WIDTH_RECT + 4, SPRITE_HEIGTH_RECT + 4, DARK_GREY);
        }

        if (sel) {
            last_roll  = roll;
            last_pitch = pitch;
        }

        drawScale(sel);

        if (instrumentType == ROUND_SHAPE) {
            tft.pushImageDMA(SPRITE_X0_ROUND, SPRITE_Y0_ROUND + (SPRITE_DIM_RADIUS)*sel, SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS, sprPtr[sel]);
            // spr[sel].pushSprite(SPRITE_X0_ROUND, SPRITE_Y0_ROUND + (SPRITE_DIM_RADIUS)*sel, TFT_TRANSPARENT);
        }
        if (instrumentType == RECT_SHAPE) {
            tft.pushImageDMA(SPRITE_X0_RECT, SPRITE_Y0_RECT + (SPRITE_HEIGTH_RECT / 2) * sel, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, sprPtr[sel]);
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
            spr[sel].fillRect(INSTRUMENT_CENTER_X0_ROUND - 1, INSTRUMENT_CENTER_Y0_ROUND - 1, 3, 3, TFT_RED);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 30, INSTRUMENT_CENTER_Y0_ROUND, 24, TFT_RED);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND + 30 - 24, INSTRUMENT_CENTER_Y0_ROUND, 24, TFT_RED);
            spr[sel].drawFastVLine(INSTRUMENT_CENTER_X0_ROUND - 30 + 24, INSTRUMENT_CENTER_Y0_ROUND, 3, TFT_RED);
            spr[sel].drawFastVLine(INSTRUMENT_CENTER_X0_ROUND + 30 - 24, INSTRUMENT_CENTER_Y0_ROUND, 3, TFT_RED);

            // Pitch scale
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND - 40, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND - 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND - 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND - 10, 12, TFT_WHITE);

            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND + 10, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND + 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 6, INSTRUMENT_CENTER_Y0_ROUND + 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_ROUND - 12, INSTRUMENT_CENTER_Y0_ROUND + 40, 24, TFT_WHITE);

            // Pitch scale values
            spr[sel].setTextColor(TFT_WHITE);
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND + 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND + 20 - 3);
            spr[sel].print("10");

            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND - 12 - 13, INSTRUMENT_CENTER_Y0_ROUND + 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_ROUND + 12 + 1, INSTRUMENT_CENTER_Y0_ROUND + 40 - 3);
            spr[sel].print("20");

            tft.setTextColor(TFT_WHITE, TFT_BLACK); // Text with background
        }
        if (instrumentType == RECT_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            spr[sel].fillRect(INSTRUMENT_CENTER_X0_RECT - 1, INSTRUMENT_CENTER_Y0_RECT - 1, 3, 3, TFT_RED);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 30, INSTRUMENT_CENTER_Y0_RECT, 24, TFT_RED);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT + 30 - 24, INSTRUMENT_CENTER_Y0_RECT, 24, TFT_RED);
            spr[sel].drawFastVLine(INSTRUMENT_CENTER_X0_RECT - 30 + 24, INSTRUMENT_CENTER_Y0_RECT, 3, TFT_RED);
            spr[sel].drawFastVLine(INSTRUMENT_CENTER_X0_RECT + 30 - 24, INSTRUMENT_CENTER_Y0_RECT, 3, TFT_RED);

            // Pitch scale
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT - 40, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT - 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT - 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT - 10, 12, TFT_WHITE);

            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT + 10, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT + 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 6, INSTRUMENT_CENTER_Y0_RECT + 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(INSTRUMENT_CENTER_X0_RECT - 12, INSTRUMENT_CENTER_Y0_RECT + 40, 24, TFT_WHITE);

            // Pitch scale values
            spr[sel].setTextColor(TFT_WHITE);
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT + 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT + 20 - 3);
            spr[sel].print("10");

            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT - 12 - 13, INSTRUMENT_CENTER_Y0_RECT + 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(INSTRUMENT_CENTER_X0_RECT + 12 + 1, INSTRUMENT_CENTER_Y0_RECT + 40 - 3);
            spr[sel].print("20");
            tft.setTextColor(TFT_BLACK, BROWN); // Text with background
        }

        // Display justified roll value near bottom of screen
        tft.setTextDatum(MC_DATUM);                                          // Centre middle justified
        tft.setTextPadding(24);                                              // Padding width to wipe previous number
        char message[40];                                                    // buffer for message
        sprintf(message, " Roll: %4d / Pitch: %3d ", last_roll, last_pitch); // create message
        tft.drawString(message, TFT_WIDTH / 2, TFT_HEIGTH - 9, 1);
    }

    void drawOuter()
    {
        if (instrumentType == ROUND_SHAPE) {
            // fill sprite with not moving area
            TFT::fillHalfCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, SKY_BLUE, 1, 0);
            TFT::fillHalfCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, SKY_BLUE, 1, 1);
            TFT::fillHalfCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, BROWN, 0, 0);
            TFT::fillHalfCircle(INSTRUMENT_CENTER_X0_ROUND, INSTRUMENT_CENTER_Y0_ROUND, INSTRUMENT_OUTER_RADIUS, BROWN, 0, 1);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            spr[0].fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT - SPRITE_HEIGTH_RECT / 2, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, SKY_BLUE);
            spr[0].fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, BROWN);
            spr[1].fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT - SPRITE_HEIGTH_RECT / 2, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, SKY_BLUE);
            spr[1].fillRect(INSTRUMENT_CENTER_X0_RECT - SPRITE_WIDTH_RECT / 2, INSTRUMENT_CENTER_Y0_RECT, SPRITE_WIDTH_RECT, SPRITE_HEIGTH_RECT / 2, BROWN);
            // fill area outside sprite with not moving area
            tft.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT / 2, SKY_BLUE);
            tft.fillRect(0, 160, TFT_WIDTH, TFT_HEIGHT / 2, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
