
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16
// Adapted to use sprites and DMA transfer for RP2040
// additionally rect and round instrument implemented
// and adapted to LilyGo 480x480 round display on ESP32

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

#define WIDTH_RECT_INNER  200
#define HEIGTH_RECT_INNER 280                   // with this dimensions 112.5kBytes are required
#define X0_RECT_INNER     20                    // upper left x position where to plot
#define Y0_RECT_INNER     20                    // upper left y position where to plot
#define CENTER_X0_RECT    WIDTH_RECT_INNER / 2  // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_RECT    HEIGTH_RECT_INNER / 2 // y mid point in sprite for instrument, complete drawing must be inside sprite
#define WIDTH_RECT_OUTER  240                   // width of clipping area for rect instrument around CENTER_X0_RECT, if higher than Sprite dimension not considered
#define HEIGTH_RECT_OUTER 320                   // height of clipping area for rect instrument around CENTER_Y0_RECT, if higher than Sprite dimension not considered
#define CENTER_X0_ROUND   240                   // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_ROUND   240                   // y mid point in sprite for instrument, complete drawing must be inside sprite
#define OUTER_RADIUS      240                   // radius of outer part of instrument
#define INNER_RADIUS      200                   // radius of moving part of instrument
#define HOR               600                   // Horizon vector line, length must be at least sqrt(WIDTH_RECT_INNER^2 + HEIGTH_RECT_INNER^2) = 344
#define MAX_PITCH         200                   // Maximum pitch shouls be in range +/- 80 with HOR = 172, 20 steps = 10 degrees on drawn scale
#define BROWN             0xFD20                // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE          0x02B5                // 0x0318 //0x039B //0x34BF
#define DARK_RED          RED                   // 0x8000
#define DARK_GREY         BLACK                 // ILI9341_DARKGREY
#define LIGHT_GREY        BLACK                 // ILI9341_LIGHTGREY

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

    void drawCentreString(const char *buf, int x, int y, uint8_t size)
    {
        int16_t  x1, y1;
        uint16_t w, h;
        gfx->setTextSize(size);
        gfx->getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
        gfx->setCursor(x - w / 2, y - h / 2);
        gfx->print(buf);
    }

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

        // position to draw lines
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
                TFT::drawLine(posX, posY, posXE, posYE, WHITE, sel);

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
                TFT::drawLine(posX, posY, posXE, posYE, WHITE, sel);

                // draw a border around the inner moving part
                gfx->drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 0, LIGHT_GREY);
                gfx->drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 1, LIGHT_GREY);
                gfx->drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY);

                drawScale(sel);
            }
        }
        if (instrumentType == RECT_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                // draw outer part
                // left side
                gfx->fillRect(0, Y0_RECT_INNER, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) - pitch - 1, SKY_BLUE);
                gfx->fillRect(0, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) + pitch, BROWN);
                gfx->drawFastHLine(0, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, WHITE);
                // right side
                gfx->fillRect(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) - pitch - 1, SKY_BLUE);
                gfx->fillRect(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, (HEIGTH_RECT_OUTER / 2) + pitch, BROWN);
                gfx->drawFastHLine(Y0_RECT_INNER + WIDTH_RECT_INNER + 2, Y0_RECT_INNER + (HEIGTH_RECT_OUTER / 2) - pitch, Y0_RECT_INNER - 2, WHITE);
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
                TFT::drawLine(posX, posY, posXE, posYE, WHITE, sel);
                // draw the scale
                drawScale(sel);

                gfx->drawRect(X0_RECT_INNER - 1, Y0_RECT_INNER - 1, WIDTH_RECT_INNER + 2, HEIGTH_RECT_INNER + 2, DARK_GREY);
                gfx->drawRect(X0_RECT_INNER - 1, Y0_RECT_INNER - 1, WIDTH_RECT_INNER + 2, HEIGTH_RECT_INNER + 2, DARK_GREY);
                gfx->drawRect(X0_RECT_INNER - 2, Y0_RECT_INNER - 2, WIDTH_RECT_INNER + 4, HEIGTH_RECT_INNER + 4, DARK_GREY);
                gfx->drawRect(X0_RECT_INNER - 2, Y0_RECT_INNER - 2, WIDTH_RECT_INNER + 4, HEIGTH_RECT_INNER + 4, DARK_GREY);
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
            gfx->fillRect(CENTER_X0_ROUND - 1, CENTER_Y0_ROUND - 1, 3, 3, RED);
            gfx->drawFastHLine(CENTER_X0_ROUND - 30, CENTER_Y0_ROUND, 24, RED);
            gfx->drawFastHLine(CENTER_X0_ROUND + 30 - 24, CENTER_Y0_ROUND, 24, RED);
            gfx->drawFastVLine(CENTER_X0_ROUND - 30 + 24, CENTER_Y0_ROUND, 3, RED);
            gfx->drawFastVLine(CENTER_X0_ROUND + 30 - 24, CENTER_Y0_ROUND, 3, RED);

            // Pitch scale
            gfx->drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND - 40, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND - 30, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND - 20, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND - 10, 12, WHITE);

            gfx->drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND + 10, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND + 20, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 6, CENTER_Y0_ROUND + 30, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_ROUND - 12, CENTER_Y0_ROUND + 40, 24, WHITE);

            // Pitch scale values
            gfx->setTextColor(WHITE);
            gfx->setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND - 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND - 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND + 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND + 20 - 3);
            gfx->print("10");

            gfx->setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND - 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND - 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_ROUND - 12 - 13, CENTER_Y0_ROUND + 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_ROUND + 12 + 1, CENTER_Y0_ROUND + 40 - 3);
            gfx->print("20");

            gfx->setTextColor(WHITE, BLACK); // Text with background
        }
        if (instrumentType == RECT_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            gfx->fillRect(CENTER_X0_RECT - 1, CENTER_Y0_RECT - 1, 3, 3, RED);
            gfx->drawFastHLine(CENTER_X0_RECT - 30, CENTER_Y0_RECT, 24, RED);
            gfx->drawFastHLine(CENTER_X0_RECT + 30 - 24, CENTER_Y0_RECT, 24, RED);
            gfx->drawFastVLine(CENTER_X0_RECT - 30 + 24, CENTER_Y0_RECT, 3, RED);
            gfx->drawFastVLine(CENTER_X0_RECT + 30 - 24, CENTER_Y0_RECT, 3, RED);

            // Pitch scale
            gfx->drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT - 40, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT - 30, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT - 20, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT - 10, 12, WHITE);

            gfx->drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT + 10, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT + 20, 24, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 6, CENTER_Y0_RECT + 30, 12, WHITE);
            gfx->drawFastHLine(CENTER_X0_RECT - 12, CENTER_Y0_RECT + 40, 24, WHITE);

            // Pitch scale values
            gfx->setTextColor(WHITE);
            gfx->setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT - 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT - 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT + 20 - 3);
            gfx->print("10");
            gfx->setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT + 20 - 3);
            gfx->print("10");

            gfx->setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT - 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT - 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_RECT - 12 - 13, CENTER_Y0_RECT + 40 - 3);
            gfx->print("20");
            gfx->setCursor(CENTER_X0_RECT + 12 + 1, CENTER_Y0_RECT + 40 - 3);
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
            TFT::fillHalfCircleTFT(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, SKY_BLUE, BROWN);
            TFT::fillHalfCircleTFT(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, SKY_BLUE, BROWN);
            // and now the "static" area
            TFT::fillHalfCircleTFT(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, SKY_BLUE, BROWN);
            gfx->drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            gfx->fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, SKY_BLUE);
            gfx->fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, BROWN);
            gfx->fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, SKY_BLUE);
            gfx->fillRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2, CENTER_Y0_RECT, WIDTH_RECT_INNER, HEIGTH_RECT_INNER / 2, BROWN);
            // fill area outside sprite with not moving area
            gfx->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT / 2, SKY_BLUE);
            gfx->fillRect(0, 160, TFT_WIDTH, TFT_HEIGHT / 2, BROWN);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
