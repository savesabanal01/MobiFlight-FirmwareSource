
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
#define SPRITE_WIDTH  240
#define SPRITE_HEIGTH 240
// with this dimensions 112.5kBytes are required
#define SPRITE_X0       0      // upper left x position where to plot
#define SPRITE_Y0       40     // upper left y position where to plot
#define CLIPPING_X0     120    // x mid point in sprite of instrument
#define CLIPPING_Y0     120    // y mid point in sprite of instrument
#define CLIPPING_XWIDTH 200    // width of clipping area for rect instrument
#define CLIPPING_YWIDTH 200    // height of clipping area for rect instrument
#define CLIPPING_R      100    // radius of clipping area for round instrument (rotating part)
#define BROWN           0xFD20 // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE        0x02B5 // 0x0318 //0x039B //0x34BF
#define DARK_RED        0x8000
#define DARK_GREY       0x39C7
#define XC              120
#define YC              120
#define HOR             400 // 130 //400 // Horizon vector line length ### was 172
#define DEG2RAD         0.0174532925
#define WAIT            10 // Pause in milliseconds to set refresh speed

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
            TFT::setClippingArea(CLIPPING_X0, CLIPPING_Y0, CLIPPING_XWIDTH, CLIPPING_YWIDTH, CLIPPING_R, CLIPPING_R);
        if (instrumentType == RECT_SHAPE)
            TFT::setClippingArea(CLIPPING_X0, CLIPPING_Y0, CLIPPING_XWIDTH, CLIPPING_YWIDTH, 0, CLIPPING_R);

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
        tft.drawString("Demo Attitude Indicator", XC, 1, 1);
        tft.drawString("Based on Bodmer's example", XC, 10, 1);
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
        // pitch = 10; //random(2 * YC) - YC;
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
            TFT::drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN, sel);
            xdn = 5 * xd;
            ydn = 5 * yd;
            TFT::drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN, sel);
            xdn = 4 * xd;
            ydn = 4 * yd;
            TFT::drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN, sel);

            xdn = 3 * xd;
            ydn = 3 * yd;
            TFT::drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE, sel);
            TFT::drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN, sel);
        }
        xdn = 2 * xd;
        ydn = 2 * yd;
        TFT::drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE, sel);
        TFT::drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN, sel);

        TFT::drawLine(XC - x0 - xd, YC - y0 - yd - pitch, XC + x0 - xd, YC + y0 - yd - pitch, SKY_BLUE, sel);
        TFT::drawLine(XC - x0 + xd, YC - y0 + yd - pitch, XC + x0 + xd, YC + y0 + yd - pitch, BROWN, sel);

        TFT::drawLine(XC - x0, YC - y0 - pitch, XC + x0, YC + y0 - pitch, TFT_WHITE, sel);

        if (sel) {
            last_roll  = roll;
            last_pitch = pitch;
        }

        if (instrumentType == ROUND_SHAPE) {
            spr[0].drawCircle(CLIPPING_X0, CLIPPING_Y0, CLIPPING_R, DARK_GREY);
            spr[1].drawCircle(CLIPPING_X0, CLIPPING_Y0, CLIPPING_R, DARK_GREY);
            spr[0].drawCircle(CLIPPING_X0, CLIPPING_Y0, CLIPPING_R + 1, DARK_GREY);
            spr[1].drawCircle(CLIPPING_X0, CLIPPING_Y0, CLIPPING_R + 1, DARK_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            spr[0].drawRect(CLIPPING_X0 - CLIPPING_XWIDTH / 2, CLIPPING_Y0 - CLIPPING_YWIDTH / 2, CLIPPING_XWIDTH + 1, CLIPPING_YWIDTH + 1, DARK_GREY);
            spr[1].drawRect(CLIPPING_X0 - CLIPPING_XWIDTH / 2, CLIPPING_Y0 - CLIPPING_YWIDTH / 2, CLIPPING_XWIDTH + 1, CLIPPING_YWIDTH + 1, DARK_GREY);
            // ToDo: Why are there sometimes some Pixel outside the area???
            spr[0].drawRect(CLIPPING_X0 - CLIPPING_XWIDTH / 2 - 1, CLIPPING_Y0 - CLIPPING_YWIDTH / 2 - 1, CLIPPING_XWIDTH + 3, CLIPPING_YWIDTH, DARK_GREY);
            spr[1].drawRect(CLIPPING_X0 - CLIPPING_XWIDTH / 2 - 1, CLIPPING_Y0 - CLIPPING_YWIDTH / 2 - 1, CLIPPING_XWIDTH + 3, CLIPPING_YWIDTH, DARK_GREY);
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
        spr[sel].fillRect(XC - 1, YC - 1, 3, 3, TFT_RED);
        spr[sel].drawFastHLine(XC - 30, YC, 24, TFT_RED);
        spr[sel].drawFastHLine(XC + 30 - 24, YC, 24, TFT_RED);
        spr[sel].drawFastVLine(XC - 30 + 24, YC, 3, TFT_RED);
        spr[sel].drawFastVLine(XC + 30 - 24, YC, 3, TFT_RED);

        // Pitch scale
        spr[sel].drawFastHLine(XC - 12, YC - 40, 24, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 6, YC - 30, 12, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 12, YC - 20, 24, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 6, YC - 10, 12, TFT_WHITE);

        spr[sel].drawFastHLine(XC - 6, YC + 10, 12, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 12, YC + 20, 24, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 6, YC + 30, 12, TFT_WHITE);
        spr[sel].drawFastHLine(XC - 12, YC + 40, 24, TFT_WHITE);

        // Pitch scale values
        spr[sel].setTextColor(TFT_WHITE);
        spr[sel].setCursor(XC - 12 - 13, YC - 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(XC + 12 + 1, YC - 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(XC - 12 - 13, YC + 20 - 3);
        spr[sel].print("10");
        spr[sel].setCursor(XC + 12 + 1, YC + 20 - 3);
        spr[sel].print("10");

        spr[sel].setCursor(XC - 12 - 13, YC - 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(XC + 12 + 1, YC - 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(XC - 12 - 13, YC + 40 - 3);
        spr[sel].print("20");
        spr[sel].setCursor(XC + 12 + 1, YC + 40 - 3);
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
        tft.drawString(message, XC, TFT_HEIGTH - 9, 1);
    }

    void drawOuter()
    {
        if (instrumentType == ROUND_SHAPE) {
            // fill sprite with not moving area
            TFT::fillHalfCircle(CLIPPING_X0, CLIPPING_Y0, SPRITE_WIDTH / 2, SKY_BLUE, 1, 0);
            TFT::fillHalfCircle(CLIPPING_X0, CLIPPING_Y0, SPRITE_WIDTH / 2, SKY_BLUE, 1, 1);
            TFT::fillHalfCircle(CLIPPING_X0, CLIPPING_Y0, SPRITE_WIDTH / 2, BROWN, 0, 0);
            TFT::fillHalfCircle(CLIPPING_X0, CLIPPING_Y0, SPRITE_WIDTH / 2, BROWN, 0, 1);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            spr[0].fillRect(XC - SPRITE_WIDTH / 2, YC - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
            spr[0].fillRect(XC - SPRITE_WIDTH / 2, YC, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);
            spr[1].fillRect(XC - SPRITE_WIDTH / 2, YC - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
            spr[1].fillRect(XC - SPRITE_WIDTH / 2, YC, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);
            // fill area outside sprite with not moving area
            tft.fillRect(0, 0, TFT_WIDTH, 40, SKY_BLUE);
            tft.fillRect(0, 280, TFT_WIDTH, 40, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
