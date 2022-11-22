
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
#define WIDTH_RECT_INNER  200
#define HEIGTH_RECT_INNER 280                   // with this dimensions 112.5kBytes are required
#define X0_RECT_INNER     20                    // upper left x position where to plot
#define Y0_RECT_INNER     20                    // upper left y position where to plot
#define CENTER_X0_RECT    WIDTH_RECT_INNER / 2  // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_RECT    HEIGTH_RECT_INNER / 2 // y mid point in sprite for instrument, complete drawing must be inside sprite
#define WIDTH_RECT_OUTER  240                   // width of outer part of instrument
#define HEIGTH_RECT_OUTER 320                   // height of outer part of instrument
#define SPRITE_DIM_RADIUS 120                   // dimension for x and y direction of sprite, including outer part
#define SPRITE_X0_ROUND   0                     // upper left x position where to plot
#define SPRITE_Y0_ROUND   40                    // upper left y position where to plot
#define CENTER_X0_ROUND   SPRITE_DIM_RADIUS     // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_ROUND   SPRITE_DIM_RADIUS     // y mid point in sprite for instrument, complete drawing must be inside sprite
#define OUTER_RADIUS      SPRITE_DIM_RADIUS     // radius of outer part of instrument
#define INNER_RADIUS      99                    // radius of moving part of instrument
#define HOR               350                   // Horizon vector line, length must be at least sqrt(WIDTH_RECT_INNER^2 + HEIGTH_RECT_INNER^2) = 344
#define MAX_PITCH         100                   // Maximum pitch shouls be in range +/- 80 with HOR = 172, 20 steps = 10 degrees on drawn scale
#define BROWN             0xFD20                // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE          0x02B5                // 0x0318 //0x039B //0x34BF
#define DARK_RED          TFT_RED               // 0x8000
#define DARK_GREY         TFT_BLACK             // ILI9341_DARKGREY
#define LIGHT_GREY        TFT_BLACK             // ILI9341_LIGHTGREY
// TFT_TRANSPARENT check how to use
// spr[0].fillSprite(TFT_TRANSPARENT);
// spr[0].setColorDepth(int8_t b);
// spr[0].createSprite(70, 80);
// spr[0].fillSprite(TFT_TRANSPARENT);
// spr[0].pushSprite(x, y, TFT_TRANSPARENT);
// spr[0].getColorDepth(void);
// spr[0].deleteSprite();

#define DEG2RAD 0.0174532925

int     roll           = 0;
int     pitch          = 0;
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
        pitch          = 0;
        roll           = 0;
        last_pitch     = 0;
        last_roll      = 0;
        tft.fillScreen(TFT_BLACK);

        spr[0].setRotation(0);
        spr[1].setRotation(0);

        if (instrumentType == ROUND_SHAPE) {
            // Create the 2 sprites, each is half the size
            sprPtr[0] = (uint16_t *)spr[0].createSprite(SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS);
            sprPtr[1] = (uint16_t *)spr[1].createSprite(SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS);
            // Move the sprite 1 coordinate datum upwards half the screen height
            // so from coordinate point of view it occupies the bottom of screen
            spr[1].setViewport(0 /* X0_RECT_INNER */, -SPRITE_DIM_RADIUS, SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS * 2);
        }
        if (instrumentType == RECT_SHAPE) {
            // Create the 2 sprites, each is half the size of the screen
            sprPtr[0] = (uint16_t *)spr[0].createSprite(WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2);
            sprPtr[1] = (uint16_t *)spr[1].createSprite(WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2);
            // Move the sprite 1 coordinate datum upwards half the screen height
            // so from coordinate point of view it occupies the bottom of screen
            spr[1].setViewport(0 /* X0_RECT_INNER */, -HEIGTH_RECT_INNER / 2, WIDTH_RECT_INNER, HEIGTH_RECT_INNER);
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

    void loop()
    {
        // Roll is in degrees in range +/-180
        // roll = random(361) - 180;
        roll++;
        if (roll == 180) roll = -180;

        // Pitch is in y coord (pixel) steps, 20 steps = 10 degrees on drawn scale
        // Maximum pitch shouls be in range +/- 80 with HOR = 172
        // pitch = 10; //random(2 * CENTER_Y0_RECT) - CENTER_Y0_RECT;
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
            drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch, 1);

            last_roll  = roll;
            last_pitch = pitch;
        }
    }

    // #########################################################################
    // Draw the horizon with a new roll (angle in range -180 to +180)
    // #########################################################################
    void drawHorizon(int roll, int pitch, bool sel)
    {
        // Calculate coordinates for line start for inner moving part
        int16_t x0 = (float)cos(roll * DEG2RAD) * HOR;
        int16_t y0 = (float)sin(roll * DEG2RAD) * HOR;
        // Calculate coordinates for line start for outer part, roll has not to be considered
        int16_t x0outer = OUTER_RADIUS;
        int16_t y0outer = 0;

        // check in which direction to move
        int16_t xd  = 0;
        int16_t yd  = 1;
        int16_t xdn = 0;
        int16_t ydn = 0;

        // positions to draw lines
        int16_t posX, posY, posXE, posYE;

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
                // draw outer part, only pitch to be considered
                TFT::setClippingArea(CENTER_X0_ROUND, CENTER_Y0_ROUND, 0, 0, OUTER_RADIUS, INNER_RADIUS);
                for (uint8_t i = 3; i > 0; i--) {
                    // just go into y-direction
                    xdn = i * 0;
                    ydn = i * 1;

                    posX  = CENTER_X0_ROUND - x0outer - xdn;
                    posY  = CENTER_Y0_ROUND - y0outer - ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0outer - xdn;
                    posYE = CENTER_Y0_ROUND + y0outer - ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, SKY_BLUE, sel);

                    posX  = CENTER_X0_ROUND - x0outer + xdn;
                    posY  = CENTER_Y0_ROUND - y0outer + ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0outer + xdn;
                    posYE = CENTER_Y0_ROUND + y0outer + ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                posX  = CENTER_X0_ROUND - x0outer;
                posY  = CENTER_Y0_ROUND - y0outer - pitch;
                posXE = CENTER_X0_ROUND + x0outer;
                posYE = CENTER_Y0_ROUND + y0outer - pitch;
                TFT::drawLine(posX, posY, posXE, posYE, TFT_WHITE, sel);

                // draw inner moving part
                TFT::setClippingArea(CENTER_X0_ROUND, CENTER_Y0_ROUND, 0, 0, INNER_RADIUS, 0);
                for (uint8_t i = 6; i > 0; i--) {
                    xdn   = i * xd;
                    ydn   = i * yd;
                    posX  = CENTER_X0_ROUND - x0 - xdn;
                    posY  = CENTER_Y0_ROUND - y0 - ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0 - xdn;
                    posYE = CENTER_Y0_ROUND + y0 - ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, SKY_BLUE, sel);
                    posX  = CENTER_X0_ROUND - x0 + xdn;
                    posY  = CENTER_Y0_ROUND - y0 + ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0 + xdn;
                    posYE = CENTER_Y0_ROUND + y0 + ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                // draw the white center line
                posX  = CENTER_X0_ROUND - x0;
                posY  = CENTER_Y0_ROUND - y0 - pitch;
                posXE = CENTER_X0_ROUND + x0;
                posYE = CENTER_Y0_ROUND + y0 - pitch;
                TFT::drawLine(posX, posY, posXE, posYE, TFT_WHITE, sel);
                // draw a border around the inner moving part
                spr[sel].drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 0, LIGHT_GREY);
                spr[sel].drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 1, LIGHT_GREY);
                spr[sel].drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY);
                // draw the scale
                drawScale(sel);

                tft.pushImageDMA(SPRITE_X0_ROUND, SPRITE_Y0_ROUND + (SPRITE_DIM_RADIUS)*sel, SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS, sprPtr[sel]);
            }
        }
        if (instrumentType == RECT_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                // draw outer part
                // left side
                tft.fillRect(0, Y0_RECT_INNER, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) - pitch - 1, SKY_BLUE);
                tft.fillRect(0, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) + pitch, BROWN);
                tft.drawFastHLine(0, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, TFT_WHITE);
                // right side
                tft.fillRect(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) - pitch - 1, SKY_BLUE);
                tft.fillRect(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) + pitch, BROWN);
                tft.drawFastHLine(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, TFT_WHITE);
                // draw inner moving part
                TFT::setClippingArea(CENTER_X0_RECT, CENTER_Y0_RECT, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER, 0, 0);
                for (uint8_t i = 6; i > 0; i--) {
                    xdn   = i * xd;
                    ydn   = i * yd;
                    posX  = CENTER_X0_RECT - x0 - xdn;
                    posY  = CENTER_Y0_RECT - y0 - ydn - pitch;
                    posXE = CENTER_X0_RECT + x0 - xdn;
                    posYE = CENTER_Y0_RECT + y0 - ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, SKY_BLUE, sel);
                    posX  = CENTER_X0_RECT - x0 + xdn;
                    posY  = CENTER_Y0_RECT - y0 + ydn - pitch;
                    posXE = CENTER_X0_RECT + x0 + xdn;
                    posYE = CENTER_Y0_RECT + y0 + ydn - pitch;
                    TFT::drawLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                // draw the white center line
                posX  = CENTER_X0_RECT - x0;
                posY  = CENTER_Y0_RECT - y0 - pitch;
                posXE = CENTER_X0_RECT + x0;
                posYE = CENTER_Y0_RECT + y0 - pitch;
                TFT::drawLine(posX, posY, posXE, posYE, TFT_WHITE, sel);
                // draw the scale
                drawScale(sel);

                tft.pushImageDMA(X0_RECT_INNER, Y0_RECT_INNER + (HEIGTH_RECT_INNER / 2) * sel, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, sprPtr[sel]);
                // draw a border around the inner moving part
                tft.drawRect(X0_RECT_INNER - 1, Y0_RECT_INNER - 1, WIDTH_RECT_INNER + 2, HEIGTH_RECT_INNER + 2, DARK_GREY);
                tft.drawRect(X0_RECT_INNER - 1, Y0_RECT_INNER - 1, WIDTH_RECT_INNER + 2, HEIGTH_RECT_INNER + 2, DARK_GREY);
                tft.drawRect(X0_RECT_INNER - 2, Y0_RECT_INNER - 2, WIDTH_RECT_INNER + 4, HEIGTH_RECT_INNER + 4, DARK_GREY);
                tft.drawRect(X0_RECT_INNER - 2, Y0_RECT_INNER - 2, WIDTH_RECT_INNER + 4, HEIGTH_RECT_INNER + 4, DARK_GREY);
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
            spr[sel].fillRect(CENTER_X0_ROUND - 1, CENTER_Y0_ROUND - 1, 3, 3, TFT_RED);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 30, CENTER_Y0_ROUND, 24, TFT_RED);
            spr[sel].drawFastHLine(CENTER_X0_ROUND + 30 - 24, CENTER_Y0_ROUND, 24, TFT_RED);
            spr[sel].drawFastVLine(CENTER_X0_ROUND - 30 + 24, CENTER_Y0_ROUND, 3, TFT_RED);
            spr[sel].drawFastVLine(CENTER_X0_ROUND + 30 - 24, CENTER_Y0_ROUND, 3, TFT_RED);

            // Pitch scale
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND - 40, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND - 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND - 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND - 10, 12, TFT_WHITE);

            spr[sel].drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND + 10, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND + 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND + 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND + 40, 24, TFT_WHITE);

            // Pitch scale values
            spr[sel].setTextColor(TFT_WHITE);
            spr[sel].setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND + 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND + 20 - 3);
            spr[sel].print("10");

            spr[sel].setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND + 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND + 40 - 3);
            spr[sel].print("20");

            tft.setTextColor(TFT_WHITE, TFT_BLACK); // Text with background
        }
        if (instrumentType == RECT_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            spr[sel].fillRect(CENTER_X0_RECT - 1, CENTER_Y0_RECT - 1, 3, 3, TFT_RED);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 30, CENTER_Y0_RECT, 24, TFT_RED);
            spr[sel].drawFastHLine(CENTER_X0_RECT + 30 - 24, CENTER_Y0_RECT, 24, TFT_RED);
            spr[sel].drawFastVLine(CENTER_X0_RECT - 30 + 24, CENTER_Y0_RECT, 3, TFT_RED);
            spr[sel].drawFastVLine(CENTER_X0_RECT + 30 - 24, CENTER_Y0_RECT, 3, TFT_RED);

            // Pitch scale
            spr[sel].drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT - 40, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT - 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT - 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT - 10, 12, TFT_WHITE);

            spr[sel].drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT + 10, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT + 20, 24, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT + 30, 12, TFT_WHITE);
            spr[sel].drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT + 40, 24, TFT_WHITE);

            // Pitch scale values
            spr[sel].setTextColor(TFT_WHITE);
            spr[sel].setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT - 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT + 20 - 3);
            spr[sel].print("10");
            spr[sel].setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT + 20 - 3);
            spr[sel].print("10");

            spr[sel].setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT - 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT + 40 - 3);
            spr[sel].print("20");
            spr[sel].setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT + 40 - 3);
            spr[sel].print("20");
            tft.setTextColor(TFT_BLACK, BROWN); // Text with background
        }

        // Display justified roll value near bottom of screen
        tft.setTextDatum(MC_DATUM);                                          // Centre middle justified
        tft.setTextPadding(24);                                              // Padding width to wipe previous number
        char message[40];                                                    // buffer for message
        sprintf(message, " Roll: %4d / Pitch: %3d ", last_roll, last_pitch); // create message
        tft.drawString(message, TFT_WIDTH / 2, TFT_HEIGHT - 9, 1);
    }

    void drawOuter()
    {
        if (instrumentType == ROUND_SHAPE) {
            // fill sprite with not moving area
            TFT::fillHalfCircleSprite(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, SKY_BLUE, BROWN, 0);
            TFT::fillHalfCircleSprite(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, SKY_BLUE, BROWN, 1);
            // and now the "static" area
            TFT::fillHalfCircleTFT(SPRITE_X0_ROUND + SPRITE_DIM_RADIUS, SPRITE_Y0_ROUND + SPRITE_DIM_RADIUS, OUTER_RADIUS, SKY_BLUE, BROWN);
            tft.drawCircle(SPRITE_X0_ROUND + SPRITE_DIM_RADIUS, SPRITE_Y0_ROUND + SPRITE_DIM_RADIUS, OUTER_RADIUS, LIGHT_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            spr[0].fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, SKY_BLUE);
            spr[0].fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, BROWN);
            spr[1].fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, SKY_BLUE);
            spr[1].fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, BROWN);
            // fill area outside sprite with not moving area
            tft.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT / 2, SKY_BLUE);
            tft.fillRect(0, 160, TFT_WIDTH, TFT_HEIGHT / 2, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
