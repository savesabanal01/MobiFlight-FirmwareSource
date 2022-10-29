
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
#define SPRITE_WIDTH    240
#define SPRITE_HEIGTH   280               // with this dimensions 112.5kBytes are required
#define SPRITE_X0       0                 // upper left x position where to plot
#define SPRITE_Y0       20                // upper left y position where to plot
#define CENTER_X0       SPRITE_WIDTH / 2  // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0       SPRITE_HEIGTH / 2 // y mid point in sprite for instrument, complete drawing must be inside sprite
#define CLIPPING_XWIDTH 200               // width of clipping area for rect instrument around CENTER_X0
#define CLIPPING_YWIDTH 200               // height of clipping area for rect instrument around CENTER_Y0
#define CLIPPING_RADIUS 100               // radius of clipping area for round instrument (rotating part) around CENTER_X0/ around CENTER_Y0
#define OUTER_WIDTH     240               // width of outer part of instrument
#define OUTER_HEIGHT    280               // height of outer part of instrument
#define HOR             400               // Horizon vector line, length must be at least sqrt(SPRITE_WIDTH^2 + SPRITE_HEIGTH^2)
#define BROWN           0xFD20            // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE        0x02B5            // 0x0318 //0x039B //0x34BF
#define DARK_RED        0x8000
#define DARK_GREY       0x39C7
#define DEG2RAD         0.0174532925

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
            TFT::setClippingArea(CENTER_X0, CENTER_Y0, 0, 0, CLIPPING_RADIUS);
        if (instrumentType == RECT_SHAPE)
            TFT::setClippingArea(CENTER_X0, CENTER_Y0, CLIPPING_XWIDTH, CLIPPING_YWIDTH, 0);

        spr[0].setRotation(0);
        spr[1].setRotation(0);

        // Create the 2 sprites, each is half the size of the screen
        sprPtr[0] = (uint16_t *)spr[0].createSprite(SPRITE_WIDTH, SPRITE_HEIGTH / 2);
        sprPtr[1] = (uint16_t *)spr[1].createSprite(SPRITE_WIDTH, SPRITE_HEIGTH / 2);
        // Move the sprite 1 coordinate datum upwards half the screen height
        // so from coordinate point of view it occupies the bottom of screen
        spr[1].setViewport(0 /* SPRITE_X0 */, -SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH);

        // Define text datum for each Sprite
        spr[0].setTextDatum(MC_DATUM);
        spr[1].setTextDatum(MC_DATUM);

        tft.startWrite(); // TFT chip select held low permanently

        // draw outer part of instrument
        drawOuter();

        // Draw fixed text at top/bottom of screen
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(TC_DATUM); // Centre middle justified
        tft.drawString("Demo Attitude Indicator", CENTER_X0, 1, 1);
        tft.drawString("Based on Bodmer's example", CENTER_X0, 10, 1);
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
        // pitch = 10; //random(2 * CENTER_Y0) - CENTER_Y0;
        pitch++;
        if (pitch > 40) pitch = -30;

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

        if ((roll != last_roll) || (pitch != last_pitch)) {
            xdn = 6 * xd;
            ydn = 6 * yd;
            TFT::drawLine(CENTER_X0 - x0 - xdn, CENTER_Y0 - y0 - ydn - pitch, CENTER_X0 + x0 - xdn, CENTER_Y0 + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(CENTER_X0 - x0 + xdn, CENTER_Y0 - y0 + ydn - pitch, CENTER_X0 + x0 + xdn, CENTER_Y0 + y0 + ydn - pitch, BROWN, sel);
            xdn = 5 * xd;
            ydn = 5 * yd;
            TFT::drawLine(CENTER_X0 - x0 - xdn, CENTER_Y0 - y0 - ydn - pitch, CENTER_X0 + x0 - xdn, CENTER_Y0 + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(CENTER_X0 - x0 + xdn, CENTER_Y0 - y0 + ydn - pitch, CENTER_X0 + x0 + xdn, CENTER_Y0 + y0 + ydn - pitch, BROWN, sel);
            xdn = 4 * xd;
            ydn = 4 * yd;
            TFT::drawLine(CENTER_X0 - x0 - xdn, CENTER_Y0 - y0 - ydn - pitch, CENTER_X0 + x0 - xdn, CENTER_Y0 + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(CENTER_X0 - x0 + xdn, CENTER_Y0 - y0 + ydn - pitch, CENTER_X0 + x0 + xdn, CENTER_Y0 + y0 + ydn - pitch, BROWN, sel);

            xdn = 3 * xd;
            ydn = 3 * yd;
            TFT::drawLine(CENTER_X0 - x0 - xdn, CENTER_Y0 - y0 - ydn - pitch, CENTER_X0 + x0 - xdn, CENTER_Y0 + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(CENTER_X0 - x0 + xdn, CENTER_Y0 - y0 + ydn - pitch, CENTER_X0 + x0 + xdn, CENTER_Y0 + y0 + ydn - pitch, BROWN, sel);
        }
        xdn = 2 * xd;
        ydn = 2 * yd;
        TFT::drawLine(CENTER_X0 - x0 - xdn, CENTER_Y0 - y0 - ydn - pitch, CENTER_X0 + x0 - xdn, CENTER_Y0 + y0 - ydn - pitch, SKY_BLUE, sel);
        TFT::drawLine(CENTER_X0 - x0 + xdn, CENTER_Y0 - y0 + ydn - pitch, CENTER_X0 + x0 + xdn, CENTER_Y0 + y0 + ydn - pitch, BROWN, sel);

        TFT::drawLine(CENTER_X0 - x0 - xd, CENTER_Y0 - y0 - yd - pitch, CENTER_X0 + x0 - xd, CENTER_Y0 + y0 - yd - pitch, SKY_BLUE, sel);
        TFT::drawLine(CENTER_X0 - x0 + xd, CENTER_Y0 - y0 + yd - pitch, CENTER_X0 + x0 + xd, CENTER_Y0 + y0 + yd - pitch, BROWN, sel);

        TFT::drawLine(CENTER_X0 - x0, CENTER_Y0 - y0 - pitch, CENTER_X0 + x0, CENTER_Y0 + y0 - pitch, TFT_WHITE, sel);

        if (sel) {
            last_roll  = roll;
            last_pitch = pitch;
        }

        if (instrumentType == ROUND_SHAPE) {
            spr[0].drawCircle(CENTER_X0, CENTER_Y0, CLIPPING_RADIUS, DARK_GREY);
            spr[1].drawCircle(CENTER_X0, CENTER_Y0, CLIPPING_RADIUS, DARK_GREY);
            spr[0].drawCircle(CENTER_X0, CENTER_Y0, CLIPPING_RADIUS + 1, DARK_GREY);
            spr[1].drawCircle(CENTER_X0, CENTER_Y0, CLIPPING_RADIUS + 1, DARK_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            spr[0].drawRect(CENTER_X0 - CLIPPING_XWIDTH / 2, CENTER_Y0 - CLIPPING_YWIDTH / 2, CLIPPING_XWIDTH, CLIPPING_YWIDTH, DARK_GREY);
            spr[1].drawRect(CENTER_X0 - CLIPPING_XWIDTH / 2, CENTER_Y0 - CLIPPING_YWIDTH / 2, CLIPPING_XWIDTH, CLIPPING_YWIDTH, DARK_GREY);
            // ToDo: Why are there sometimes some Pixel outside the area???
            spr[0].drawRect(CENTER_X0 - CLIPPING_XWIDTH / 2 - 1, CENTER_Y0 - CLIPPING_YWIDTH / 2 - 1, CLIPPING_XWIDTH + 2, CLIPPING_YWIDTH + 2, DARK_GREY);
            spr[1].drawRect(CENTER_X0 - CLIPPING_XWIDTH / 2 - 1, CENTER_Y0 - CLIPPING_YWIDTH / 2 - 1, CLIPPING_XWIDTH + 2, CLIPPING_YWIDTH + 2, DARK_GREY);
        }

        drawScale(sel);

        tft.pushImageDMA(SPRITE_X0, SPRITE_Y0 + (SPRITE_HEIGTH / 2) * sel, SPRITE_WIDTH, SPRITE_HEIGTH / 2, sprPtr[sel]);
    }

    // #########################################################################
    // Draw the information
    // #########################################################################
    void drawScale(bool sel)
    {
        // Update things near middle of screen first (most likely to get obscured)

        // Level wings graphic
        spr[sel].fillRect(CENTER_X0 - 1, CENTER_Y0 - 1, 3, 3, TFT_RED);
        spr[sel].drawFastHLine(CENTER_X0 - 30, CENTER_Y0, 24, TFT_RED);
        spr[sel].drawFastHLine(CENTER_X0 + 30 - 24, CENTER_Y0, 24, TFT_RED);
        spr[sel].drawFastVLine(CENTER_X0 - 30 + 24, CENTER_Y0, 3, TFT_RED);
        spr[sel].drawFastVLine(CENTER_X0 + 30 - 24, CENTER_Y0, 3, TFT_RED);

        // Pitch scale
        spr[sel].drawFastHLine(CENTER_X0 - 12, CENTER_Y0 - 40, 24, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 6, CENTER_Y0 - 30, 12, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 12, CENTER_Y0 - 20, 24, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 6, CENTER_Y0 - 10, 12, TFT_WHITE);

        spr[sel].drawFastHLine(CENTER_X0 - 6, CENTER_Y0 + 10, 12, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 12, CENTER_Y0 + 20, 24, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 6, CENTER_Y0 + 30, 12, TFT_WHITE);
        spr[sel].drawFastHLine(CENTER_X0 - 12, CENTER_Y0 + 40, 24, TFT_WHITE);

        // Pitch scale values
        spr[sel].setTextColor(TFT_WHITE);
        spr[sel].setCursor(CENTER_X0 - 12 - 13, CENTER_Y0 - 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(CENTER_X0 + 12 + 1, CENTER_Y0 - 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(CENTER_X0 - 12 - 13, CENTER_Y0 + 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(CENTER_X0 + 12 + 1, CENTER_Y0 + 20 - 3);
        spr[sel].print("10");

        spr[sel].setCursor(CENTER_X0 - 12 - 13, CENTER_Y0 - 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(CENTER_X0 + 12 + 1, CENTER_Y0 - 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(CENTER_X0 - 12 - 13, CENTER_Y0 + 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(CENTER_X0 + 12 + 1, CENTER_Y0 + 40 - 3);
        spr[sel].print("20");

        // Display justified roll value near bottom of screen
        if (instrumentType == ROUND_SHAPE)
            tft.setTextColor(TFT_WHITE, TFT_BLACK); // Text with background
        else
            tft.setTextColor(TFT_BLACK, BROWN);                              // Text with background
        tft.setTextDatum(MC_DATUM);                                          // Centre middle justified
        tft.setTextPadding(24);                                              // Padding width to wipe previous number
        char message[40];                                                    // buffer for message
        sprintf(message, " Roll: %4d / Pitch: %3d ", last_roll, last_pitch); // create message
        tft.drawString(message, CENTER_X0, TFT_HEIGTH - 9, 1);
    }

    void drawOuter()
    {
        if (instrumentType == ROUND_SHAPE) {
            // fill sprite with not moving area
            TFT::fillHalfCircle(CENTER_X0, CENTER_Y0, SPRITE_WIDTH / 2, SKY_BLUE, 1, 0);
            TFT::fillHalfCircle(CENTER_X0, CENTER_Y0, SPRITE_WIDTH / 2, SKY_BLUE, 1, 1);
            TFT::fillHalfCircle(CENTER_X0, CENTER_Y0, SPRITE_WIDTH / 2, BROWN, 0, 0);
            TFT::fillHalfCircle(CENTER_X0, CENTER_Y0, SPRITE_WIDTH / 2, BROWN, 0, 1);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            spr[0].fillRect(CENTER_X0 - SPRITE_WIDTH / 2, CENTER_Y0 - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
            spr[0].fillRect(CENTER_X0 - SPRITE_WIDTH / 2, CENTER_Y0, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);
            spr[1].fillRect(CENTER_X0 - SPRITE_WIDTH / 2, CENTER_Y0 - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
            spr[1].fillRect(CENTER_X0 - SPRITE_WIDTH / 2, CENTER_Y0, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);
            // fill area outside sprite with not moving area
            tft.fillRect(0, 0, TFT_WIDTH, 40, SKY_BLUE);
            tft.fillRect(0, 280, TFT_WIDTH, 40, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
