
// Demo code for artifical horizon display
// Written by Bodmer for a 160 x 128 TFT display
// 16/8/16

#include <Arduino.h>
#include "TFT.h"
#include "AttitudeIndicator.h"

// Define the width and height according to the TFT and the
// available memory. The sprites will require:
//     SPRITE_WIDTH * SPRITE_HEIGTH * 2 bytes of RAM
// Note: for a 240 * 320 area this is 150 Kbytes!
#define SPRITE_WIDTH  240
#define SPRITE_HEIGTH 240
#define SPRITE_X0     0  // upper left x position where to plot
#define SPRITE_Y0     40 // upper left y position where to plot
#define TFT_WIDTH     240
#define TFT_HEIGTH    320
#define REDRAW_DELAY  16     // minimum delay in milliseconds between display updates
#define BROWN         0xFD20 // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE      0x02B5 // 0x0318 //0x039B //0x34BF
#define DARK_RED      0x8000
#define DARK_GREY     0x39C7
#define XC            120
#define YC            120
#define HOR           400 // 130 //400 // Horizon vector line length ### was 172
#define DEG2RAD       0.0174532925
#define WAIT          10 // Pause in milliseconds to set refresh speed

void updateHorizon(int roll, int pitch);
void drawHorizon(int roll, int pitch, bool sel);
int  rollGenerator(int maxroll);
void testRoll(void);
void testPitch(void);
void drawScale(bool sel);
void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color, bool sel);
void drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color, bool sel);
void drawFastVLine(int32_t x, int32_t y, int32_t w, uint32_t color, bool sel);
void drawPixel(int32_t x, int32_t y, uint32_t color, bool sel);


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
    tft.startWrite(); // TFT chip select held low permanently

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

    spr[0].fillRect(XC - SPRITE_WIDTH / 2, YC - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
    spr[0].fillRect(XC - SPRITE_WIDTH / 2, YC, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);
    spr[1].fillRect(XC - SPRITE_WIDTH / 2, YC - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH / 2, SKY_BLUE);
    spr[1].fillRect(XC - SPRITE_WIDTH / 2, YC, SPRITE_WIDTH, SPRITE_HEIGTH / 2, BROWN);

    // Draw the horizon graphic
    drawHorizon(0, 0, 0);
    drawHorizon(0, 0, 1);

    // Draw fixed text at top/bottom of screen
    tft.setTextColor(TFT_YELLOW);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("SPD  LNAV WNAV PTH", XC, 1, 1);
    tft.drawString("Bodmer's AHI", XC, TFT_HEIGTH - 9, 1);

    startMillis = millis();
}

void stop_AttitudeIndicator()
{
    // Delete sprite to free up the RAM
    spr[0].deleteSprite();
    spr[1].deleteSprite();
}

// #########################################################################
// Main loop, keeps looping around
// #########################################################################
int roll  = 0;
int pitch = 0;

void loop_AttitudeIndicator()
{
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(TC_DATUM); // Centre middle justified
    tft.drawString("Random", XC, 10, 1);

    // Refresh the display at regular intervals
    //    if (millis() > redrawTime)
    {
        redrawTime = millis() + REDRAW_DELAY;

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
void swap(int16_t *a, int16_t *b)
{
    int16_t temp = *b;
    *b           = *a;
    *a           = temp;
}

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
        spr[sel].drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
        spr[sel].drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
        xdn = 5 * xd;
        ydn = 5 * yd;
        spr[sel].drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
        spr[sel].drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
        xdn = 4 * xd;
        ydn = 4 * yd;
        spr[sel].drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
        spr[sel].drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

        xdn = 3 * xd;
        ydn = 3 * yd;
        spr[sel].drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
        spr[sel].drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);
    }
    xdn = 2 * xd;
    ydn = 2 * yd;
    spr[sel].drawLine(XC - x0 - xdn, YC - y0 - ydn - pitch, XC + x0 - xdn, YC + y0 - ydn - pitch, SKY_BLUE);
    spr[sel].drawLine(XC - x0 + xdn, YC - y0 + ydn - pitch, XC + x0 + xdn, YC + y0 + ydn - pitch, BROWN);

    spr[sel].drawLine(XC - x0 - xd, YC - y0 - yd - pitch, XC + x0 - xd, YC + y0 - yd - pitch, SKY_BLUE);
    spr[sel].drawLine(XC - x0 + xd, YC - y0 + yd - pitch, XC + x0 + xd, YC + y0 + yd - pitch, BROWN);

    spr[sel].drawLine(XC - x0, YC - y0 - pitch, XC + x0, YC + y0 - pitch, TFT_WHITE);

    if (sel) {
        last_roll  = roll;
        last_pitch = pitch;
    }
    /*
        for (uint16_t i = 99; i < 170; i++) {
           spr[sel].drawCircle(XC, YC, i, TFT_BLACK);
        }
    */
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
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);                             // Text with background
    tft.setTextDatum(MC_DATUM);                                          // Centre middle justified
    tft.setTextPadding(24);                                              // Padding width to wipe previous number
    char message[40];                                                    // buffer for message
    sprintf(message, " Roll: %4d / Pitch: %3d ", last_roll, last_pitch); // create message
    tft.drawString(message, XC, TFT_HEIGTH - 18, 1);
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

// #########################################################################
// Helper functions transferred from the lib for a round clipping area
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

  int32_t dx = x1 - x0, dy = abs(y1 - y0);;

  int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

  if (y0 < y1) ystep = 1;

  // Split into steep and not steep for FastH/V separation
  if (steep) {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        if (dlen == 1) drawPixel(y0, xs, color, sel);
        else drawFastVLine(y0, xs, dlen, color, sel);
        dlen = 0;
        y0 += ystep; xs = x0 + 1;
        err += dx;
      }
    }
    if (dlen) drawFastVLine(y0, xs, dlen, color, sel);
  }
  else
  {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        if (dlen == 1) drawPixel(xs, y0, color, sel);
        else drawFastHLine(xs, y0, dlen, color, sel);
        dlen = 0;
        y0 += ystep; xs = x0 + 1;
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
    spr[sel].drawFastHLine(x,y,w,color);
}

/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color, bool sel)
{
    spr[sel].drawFastVLine(x,y,h,color);
}


/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void drawPixel(int32_t x, int32_t y, uint32_t color, bool sel)
{

    // Range checking
    // TBD
    spr[sel].drawPixel(x,y,color);

}
