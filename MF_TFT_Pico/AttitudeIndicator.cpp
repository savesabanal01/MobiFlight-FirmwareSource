
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16
// Adapted to use sprites and DMA transfer for RP2040
// additionally rect and round instrument implemented

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

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

        TFT::initInstrument(instrumentType);
        // draw outer part of instrument
        drawOuter();
    }

    void stop()
    {
        TFT::stopInstrument();
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
        int16_t x0, y0;
        if (instrumentType == ROUND_SHAPE) {
            x0 = (float)cos(roll * DEG2RAD) * HOR_ROUND;
            y0 = (float)sin(roll * DEG2RAD) * HOR_ROUND;
        } else {
            x0 = (float)cos(roll * DEG2RAD) * HOR_RECT;
            y0 = (float)sin(roll * DEG2RAD) * HOR_RECT;
        }
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
                    TFT::drawClippedLine(posX, posY, posXE, posYE, SKY_BLUE, sel);

                    posX  = CENTER_X0_ROUND - x0outer + xdn;
                    posY  = CENTER_Y0_ROUND - y0outer + ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0outer + xdn;
                    posYE = CENTER_Y0_ROUND + y0outer + ydn - pitch;
                    TFT::drawClippedLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                posX  = CENTER_X0_ROUND - x0outer;
                posY  = CENTER_Y0_ROUND - y0outer - pitch;
                posXE = CENTER_X0_ROUND + x0outer;
                posYE = CENTER_Y0_ROUND + y0outer - pitch;
                TFT::drawClippedLine(posX, posY, posXE, posYE, TFT_WHITE, sel);

                // draw inner moving part
                TFT::setClippingArea(CENTER_X0_ROUND, CENTER_Y0_ROUND, 0, 0, INNER_RADIUS, 0);
                for (uint8_t i = 6; i > 0; i--) {
                    xdn   = i * xd;
                    ydn   = i * yd;
                    posX  = CENTER_X0_ROUND - x0 - xdn;
                    posY  = CENTER_Y0_ROUND - y0 - ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0 - xdn;
                    posYE = CENTER_Y0_ROUND + y0 - ydn - pitch;
                    TFT::drawClippedLine(posX, posY, posXE, posYE, SKY_BLUE, sel);
                    posX  = CENTER_X0_ROUND - x0 + xdn;
                    posY  = CENTER_Y0_ROUND - y0 + ydn - pitch;
                    posXE = CENTER_X0_ROUND + x0 + xdn;
                    posYE = CENTER_Y0_ROUND + y0 + ydn - pitch;
                    TFT::drawClippedLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                // draw the white center line
                posX  = CENTER_X0_ROUND - x0;
                posY  = CENTER_Y0_ROUND - y0 - pitch;
                posXE = CENTER_X0_ROUND + x0;
                posYE = CENTER_Y0_ROUND + y0 - pitch;
                TFT::drawClippedLine(posX, posY, posXE, posYE, TFT_WHITE, sel);
                // draw a border around the inner moving part
                TFT::drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 0, LIGHT_GREY, sel);
                TFT::drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, INNER_RADIUS - 1, LIGHT_GREY, sel);
                TFT::drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY, sel);
                // draw the scale
                drawScale(sel);

                tft.pushImageDMA(X0_ROUND, Y0_ROUND + (SPRITE_DIM_RADIUS)*sel, SPRITE_DIM_RADIUS * 2, SPRITE_DIM_RADIUS, sprPtr[sel]);
            }
        }
        if (instrumentType == RECT_SHAPE) {
            if ((roll != last_roll) || (pitch != last_pitch)) {
                // draw outer part
                // left side
                TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT - HEIGTH_RECT_OUTER / 2, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, HEIGTH_RECT_OUTER / 2 - pitch, SKY_BLUE, sel);
                TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT - pitch, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, HEIGTH_RECT_OUTER / 2 + pitch, BROWN, sel);
                spr[sel].drawFastHLine(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT - pitch, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, TFT_WHITE);
                // next right side
                TFT::fillRect(CENTER_X0_RECT + WIDTH_RECT_INNER / 2 + 1, CENTER_Y0_RECT - HEIGTH_RECT_OUTER / 2, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, HEIGTH_RECT_OUTER / 2 - pitch, SKY_BLUE, sel);
                TFT::fillRect(CENTER_X0_RECT + WIDTH_RECT_INNER / 2 + 1, CENTER_Y0_RECT - pitch, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, HEIGTH_RECT_OUTER / 2 + pitch, BROWN, sel);
                spr[sel].drawFastHLine(CENTER_X0_RECT + WIDTH_RECT_INNER / 2 + 1, CENTER_Y0_RECT - pitch, (WIDTH_RECT_OUTER - WIDTH_RECT_INNER) / 2 - 1, TFT_WHITE);

                // draw inner moving part
                TFT::setClippingArea(CENTER_X0_RECT, CENTER_Y0_RECT, WIDTH_RECT_INNER, HEIGTH_RECT_INNER, 0, 0);
                for (uint8_t i = 6; i > 0; i--) {
                    xdn   = i * xd;
                    ydn   = i * yd;
                    posX  = CENTER_X0_RECT - x0 - xdn;
                    posY  = CENTER_Y0_RECT - y0 - ydn - pitch;
                    posXE = CENTER_X0_RECT + x0 - xdn;
                    posYE = CENTER_Y0_RECT + y0 - ydn - pitch;
                    TFT::drawClippedLine(posX, posY, posXE, posYE, SKY_BLUE, sel);
                    posX  = CENTER_X0_RECT - x0 + xdn;
                    posY  = CENTER_Y0_RECT - y0 + ydn - pitch;
                    posXE = CENTER_X0_RECT + x0 + xdn;
                    posYE = CENTER_Y0_RECT + y0 + ydn - pitch;
                    TFT::drawClippedLine(posX, posY, posXE, posYE, BROWN, sel);
                }
                // draw the white center line
                posX  = CENTER_X0_RECT - x0;
                posY  = CENTER_Y0_RECT - y0 - pitch;
                posXE = CENTER_X0_RECT + x0;
                posYE = CENTER_Y0_RECT + y0 - pitch;
                TFT::drawClippedLine(posX, posY, posXE, posYE, TFT_WHITE, sel);
                // draw the scale
                drawScale(sel);

                // draw a border around the inner moving part
                TFT::drawRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2 - 0, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2 - 0, WIDTH_RECT_INNER + 0, HEIGTH_RECT_INNER + 0, DARK_GREY, sel);
                TFT::drawRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2 - 0, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2 - 0, WIDTH_RECT_INNER + 0, HEIGTH_RECT_INNER + 0, DARK_GREY, sel);
                TFT::drawRect(CENTER_X0_RECT - WIDTH_RECT_INNER / 2 - 1, CENTER_Y0_RECT - HEIGTH_RECT_INNER / 2 - 1, WIDTH_RECT_INNER + 2, HEIGTH_RECT_INNER + 2, DARK_GREY, sel);
                tft.pushImageDMA(X0_RECT, Y0_RECT + (HEIGTH_RECT_OUTER / 2) * sel, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER / 2, sprPtr[sel]);
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
            TFT::fillRect(CENTER_X0_ROUND - 1, CENTER_Y0_ROUND - 1, 3, 3, TFT_RED, sel);
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

            spr[sel].setTextColor(TFT_WHITE, TFT_BLACK); // Text with background
        }
        if (instrumentType == RECT_SHAPE) {
            // Update things near middle of screen first (most likely to get obscured)
            // Level wings graphic
            TFT::fillRect(CENTER_X0_RECT - 1, CENTER_Y0_RECT - 1, 3, 3, TFT_RED, sel);
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
            spr[sel].setTextColor(TFT_BLACK, BROWN); // Text with background
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
            TFT::fillHalfCircleTFT(X0_ROUND + SPRITE_DIM_RADIUS, Y0_ROUND + SPRITE_DIM_RADIUS, OUTER_RADIUS, SKY_BLUE, BROWN);
            TFT::drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY, 0);
            TFT::drawCircle(CENTER_X0_ROUND, CENTER_Y0_ROUND, OUTER_RADIUS, LIGHT_GREY, 1);
        }
        if (instrumentType == RECT_SHAPE) {
            // fill sprite with not moving area
            TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT - HEIGTH_RECT_OUTER / 2, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER / 2, SKY_BLUE, 0);
            TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER / 2, BROWN, 0);
            spr[0].drawFastHLine(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2 - 1, CENTER_Y0_RECT, WIDTH_RECT_OUTER - 2, TFT_WHITE);
            TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT - HEIGTH_RECT_OUTER / 2, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER / 2, SKY_BLUE, 1);
            TFT::fillRect(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2, CENTER_Y0_RECT, WIDTH_RECT_OUTER, HEIGTH_RECT_OUTER / 2, BROWN, 1);
            spr[1].drawFastHLine(CENTER_X0_RECT - WIDTH_RECT_OUTER / 2 - 1, CENTER_Y0_RECT, WIDTH_RECT_OUTER - 2, TFT_WHITE);
        }
        // Draw the horizon graphic
        drawHorizon(0, 0, 0);
        drawHorizon(0, 0, 1);
    }
} // end of namespace
