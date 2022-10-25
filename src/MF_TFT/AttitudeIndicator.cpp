
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

#define REDRAW_DELAY 16     // minimum delay in milliseconds between display updates
#define BROWN        ILI9341_ORANGE //0x5140 // 0x5960
#define SKY_BLUE     0x02B5 // 0x0318 //0x039B //0x34BF
#define DARK_RED     0x8000
#define DARK_GREY    0x39C7
#define XC           120 //
#define YC           160 //
#define HOR          130 //400 // Horizon vector line length ### was 172
#define DEG2RAD      0.0174532925

void updateHorizon(int roll, int pitch);
void drawHorizon(int roll, int pitch);
void drawInfo(void);
int  rollGenerator(int maxroll);
void testRoll(void);
void testPitch(void);

int last_roll  = 0; // the whole horizon graphic
int last_pitch = 0;

// Variables for test only
int test_roll = 0;
int delta     = 0;

uint32_t redrawTime = 0;

// #########################################################################
// Setup, runs once on boot up
// #########################################################################

void init_AttitudeIndicator(void)
{
    randomSeed(analogRead(A0));
    tft.startWrite(); // TFT chip select held low permanently
    
}

// #########################################################################
// Main loop, keeps looping around
// #########################################################################
int roll  = 0;
int pitch = 0;

void loop_AttitudeIndicator()
{
/*
    tft.fillRect(XC - 100, YC - 100, 200, 100, SKY_BLUE);
    tft.fillRect(XC - 100, YC, 200, 100, BROWN);
    for (uint16_t i = 99; i < 150; i++)
    {
      tft.drawCircle(XC, YC, i, ILI9341_BLACK);
    }
    // Draw the horizon graphic
    drawHorizon(0, 0);
    drawInfo();
*/
    // Test roll and pitch
    testRoll();
    testPitch();

    tft.setTextColor(TFT_YELLOW, SKY_BLUE);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("Random", XC, 10, 1);

    while (1) {
        // Refresh the display at regular intervals
        if (millis() > redrawTime) {
            redrawTime = millis() + REDRAW_DELAY;

            // Roll is in degrees in range +/-180
            // roll = random(361) - 180;
            roll++;
            if (roll == 180) roll = -180;

            // Pitch is in y coord (pixel) steps, 20 steps = 10 degrees on drawn scale
            // Maximum pitch shouls be in range +/- 80 with HOR = 172
            // pitch = 10; //random(2 * YC) - YC;
            pitch++;
            if (pitch == 30) pitch = -30;

            updateHorizon(roll, pitch);
        }
    }
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

        drawHorizon(last_roll + delta_roll, last_pitch + delta_pitch);
        drawInfo();
    }
}

// #########################################################################
// Draw the horizon with a new roll (angle in range -180 to +180)
// #########################################################################

void drawHorizon(int roll, int pitch)
{
    tft.setAddrWindow(XC - 100, YC - 100, 200, 200);
    // Calculate coordinates for line start
    float sx = cos(roll * DEG2RAD);
    float sy = sin(roll * DEG2RAD);

    int16_t x0  = sx * HOR;
    int16_t y0  = sy * HOR;
int16_t clipX = x0;
int16_t clipY = y0;
int16_t drawXS = 0;
int16_t drawYS = 0;
int16_t drawXE = 0;
int16_t drawYE = 0;

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
        drawXS = XC - x0 - xdn;
        drawYS = YC - y0 - ydn - pitch;
        drawXE = XC + x0 - xdn;
        drawYE = YC + y0 - ydn - pitch;
        if (drawXS < XC - 100) drawXS = XC - 100;
        if (drawYS < YC - 100) drawXS = YC - 100;
        if (drawXE > XC + 100) drawXE = XC + 100;
        if (drawYE > YC + 100) drawYE = XC + 100;
        tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
        
        xdn = 5 * xd;
        ydn = 5 * yd;
        drawXS = XC - x0 - xdn;
        drawYS = YC - y0 - ydn - pitch;
        drawXE = XC + x0 - xdn;
        drawYE = YC + y0 - ydn - pitch;
        if (drawXS < XC - 100) drawXS = XC - 100;
        if (drawYS < YC - 100) drawXS = YC - 100;
        if (drawXE > XC + 100) drawXE = XC + 100;
        if (drawYE > YC + 100) drawYE = XC + 100;
        tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
        
        xdn = 4 * xd;
        ydn = 4 * yd;
        drawXS = XC - x0 - xdn;
        drawYS = YC - y0 - ydn - pitch;
        drawXE = XC + x0 - xdn;
        drawYE = YC + y0 - ydn - pitch;
        if (drawXS < XC - 100) drawXS = XC - 100;
        if (drawYS < YC - 100) drawXS = YC - 100;
        if (drawXE > XC + 100) drawXE = XC + 100;
        if (drawYE > YC + 100) drawYE = XC + 100;
        tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

        xdn = 3 * xd;
        ydn = 3 * yd;
        drawXS = XC - x0 - xdn;
        drawYS = YC - y0 - ydn - pitch;
        drawXE = XC + x0 - xdn;
        drawYE = YC + y0 - ydn - pitch;
        if (drawXS < XC - 100) drawXS = XC - 100;
        if (drawYS < YC - 100) drawXS = YC - 100;
        if (drawXE > XC + 100) drawXE = XC + 100;
        if (drawYE > YC + 100) drawYE = XC + 100;
        tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
    }

    xdn = 2 * xd;
    ydn = 2 * yd;
    drawXS = XC - x0 - xdn;
    drawYS = YC - y0 - ydn - pitch;
    drawXE = XC + x0 - xdn;
    drawYE = YC + y0 - ydn - pitch;
    if (drawXS < XC - 100) drawXS = XC - 100;
    if (drawYS < YC - 100) drawXS = YC - 100;
    if (drawXE > XC + 100) drawXE = XC + 100;
    if (drawYE > YC + 100) drawYE = XC + 100;
    tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

    drawXS = XC - x0 - xd;
    drawYS = YC - y0 - yd - pitch;
    drawXE = XC + x0 - xd;
    drawYE = YC + y0 - yd - pitch;
    if (drawXS < XC - 100) drawXS = XC - 100;
    if (drawYS < YC - 100) drawXS = YC - 100;
    if (drawXE > XC + 100) drawXE = XC + 100;
    if (drawYE > YC + 100) drawYE = XC + 100;
    tft.drawLine(drawXS, drawYS, drawXE, drawYE, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

    drawXS = XC - x0;
    drawYS = YC - y0 - pitch;
    drawXE = XC + x0;
    drawYE = YC + y0 - pitch;
    if (drawXS < XC - 100) drawXS = XC - 100;
    if (drawYS < YC - 100) drawXS = YC - 100;
    if (drawXE > XC + 100) drawXE = XC + 100;
    if (drawYE > YC + 100) drawYE = XC + 100;
    tft.drawLine(XC - x0, YC - y0 - pitch,   XC + x0, YC + y0 - pitch,   TFT_WHITE);
//delay(1000);
/*
    if ((roll != last_roll) || (pitch != last_pitch)) {
        xdn = 6 * xd;
        ydn = 6 * yd;
        tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);
        xdn = 5 * xd;
        ydn = 5 * yd;
        tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);
        xdn = 4 * xd;
        ydn = 4 * yd;
        tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);

        xdn = 3 * xd;
        ydn = 3 * yd;
        tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
        tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);
    }

    xdn = 2 * xd;
    ydn = 2 * yd;

    tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);

    tft.drawLine(XC - x0 - xdn - pitchX, YC - y0 - ydn - pitch - pitchY, XC + x0 - xdn + pitchX, YC + y0 - ydn - pitch - pitchY, SKY_BLUE);
    tft.drawLine(XC - x0 + xdn - pitchX, YC - y0 + ydn - pitch - pitchY, XC + x0 + xdn + pitchX, YC + y0 + ydn - pitch - pitchY, BROWN);

    tft.drawLine(XC - x0 - pitchX, YC - y0 - pitch - pitchY,   XC + x0 + pitchX, YC + y0 - pitch - pitchY,   TFT_WHITE);
*/
    last_roll  = roll;
    last_pitch = pitch;
/*
    for (uint16_t i = 99; i < 160; i++)
    {
      tft.drawCircle(XC, YC, i, ILI9341_BLACK);
      //drawPieSlice(XC, YC, i, ILI9341_BLACK, 0, 180);
    }
*/
    tft.setAddrWindow(0, 0, DWIDTH, DHEIGHT);
}

// #########################################################################
// Draw the information
// #########################################################################

void drawInfo(void)
{
    // Update things near middle of screen first (most likely to get obscured)

    // Level wings graphic
    tft.fillRect(XC - 1, YC - 1, 3, 3, TFT_RED);
    tft.drawFastHLine(XC - 30, YC, 24, TFT_RED);
    tft.drawFastHLine(XC + 30 - 24, YC, 24, TFT_RED);
    tft.drawFastVLine(XC - 30 + 24, YC, 3, TFT_RED);
    tft.drawFastVLine(XC + 30 - 24, YC, 3, TFT_RED);

    // Pitch scale
    tft.drawFastHLine(XC - 12, YC - 40, 24, TFT_WHITE);
    tft.drawFastHLine(XC - 6, YC - 30, 12, TFT_WHITE);
    tft.drawFastHLine(XC - 12, YC - 20, 24, TFT_WHITE);
    tft.drawFastHLine(XC - 6, YC - 10, 12, TFT_WHITE);

    tft.drawFastHLine(XC - 6, YC + 10, 12, TFT_WHITE);
    tft.drawFastHLine(XC - 12, YC + 20, 24, TFT_WHITE);
    tft.drawFastHLine(XC - 6, YC + 30, 12, TFT_WHITE);
    tft.drawFastHLine(XC - 12, YC + 40, 24, TFT_WHITE);

    // Pitch scale values
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(XC - 12 - 13, YC - 20 - 3);
    tft.print("10");
    tft.setCursor(XC + 12 + 1, YC - 20 - 3);
    tft.print("10");
    tft.setCursor(XC - 12 - 13, YC + 20 - 3);
    tft.print("10");
    tft.setCursor(XC + 12 + 1, YC + 20 - 3);
    tft.print("10");

    tft.setCursor(XC - 12 - 13, YC - 40 - 3);
    tft.print("20");
    tft.setCursor(XC + 12 + 1, YC - 40 - 3);
    tft.print("20");
    tft.setCursor(XC - 12 - 13, YC + 40 - 3);
    tft.print("20");
    tft.setCursor(XC + 12 + 1, YC + 40 - 3);
    tft.print("20");

    // Display justified roll value near bottom of screen
    tft.setTextColor(TFT_YELLOW, BROWN); // Text with background
    tft.setTextDatum(MC_DATUM);          // Centre middle justified
    tft.setTextPadding(24);              // Padding width to wipe previous number
    tft.drawNumber(last_roll, XC, DHEIGHT - 18, 1);

    // Draw fixed text at bottom of screen
    tft.setTextColor(TFT_YELLOW);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("SPD  LNAV WNAV PTH", XC, 1, 1);
    tft.drawString("Bodmer's AHI", XC, DHEIGHT - 9, 1);
}

// #########################################################################
// Function to generate roll angles for testing only
// #########################################################################

int rollGenerator(int maxroll)
{
    // Synthesize a smooth +/- 50 degree roll value for testing
    delta++;
    if (delta >= 360) test_roll = 0;
    test_roll = (maxroll + 1) * sin((delta)*DEG2RAD);

    // Clip value so we hold roll near peak
    if (test_roll > maxroll) test_roll = maxroll;
    if (test_roll < -maxroll) test_roll = -maxroll;

    return test_roll;
}

// #########################################################################
// Function to generate roll angles for testing only
// #########################################################################

void testRoll(void)
{
    tft.setTextColor(TFT_YELLOW, SKY_BLUE);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("Roll test", XC, 10, 1);

    for (int a = 0; a < 360; a++) {
        // delay(REDRAW_DELAY / 2);
        updateHorizon(rollGenerator(180), 0);
    }
    tft.setTextColor(TFT_YELLOW, SKY_BLUE);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("         ", XC, 10, 1);
}

// #########################################################################
// Function to generate pitch angles for testing only
// #########################################################################

void testPitch(void)
{
    tft.setTextColor(TFT_YELLOW, SKY_BLUE);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("Pitch test", XC, 10, 1);

    for (int p = 0; p > -80; p--) {
        delay(REDRAW_DELAY / 2);
        updateHorizon(0, p);
    }

    for (int p = -80; p < 80; p++) {
        delay(REDRAW_DELAY / 2);
        updateHorizon(0, p);
    }

    for (int p = 80; p > 0; p--) {
        delay(REDRAW_DELAY / 2);
        updateHorizon(0, p);
    }

    tft.setTextColor(TFT_YELLOW, SKY_BLUE);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("          ", XC, 10, 1);
}
