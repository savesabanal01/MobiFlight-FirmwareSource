/*
  Draw a compass on the screen.

  The sketch has been tested on a 320x240 ILI9341 based TFT, it
  could be adapted for other screen sizes.

  A Sprite is notionally an invisible graphics screen that is
  kept in the processors RAM. Graphics can be drawn into the
  Sprite just as it can be drawn directly to the screen. Once
  the Sprite is completed it can be plotted onto the screen in
  any position. If there is sufficient RAM then the Sprite can
  be the same size as the screen and used as a frame buffer.

  The Sprite occupies (2 * width * height) bytes.

  On a ESP8266 Sprite sizes up to 128 x 160 can be accomodated,
  this size requires 128*160*2 bytes (40kBytes) of RAM, this must be
  available or the processor will crash. You need to make the sprite
  small enough to fit, with RAM spare for any "local variables" that
  may be needed by your sketch and libraries.

  Created by Bodmer 1/12/17

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
*/

#include <Arduino.h>
#include "TFT.h"

#define BACKGROUND      TFT_BLACK
#define SPRITE_WIDTH    160               // 100 // size of sprite
#define SPRITE_HEIGTH   160               // 100 // size pf sprite
#define SPRITE_CENTER_X 120               // center position where to plot
#define SPRITE_CENTER_Y 160               // center position where to plot
#define X0              SPRITE_WIDTH / 2  // center position of compass
#define Y0              SPRITE_HEIGTH / 2 // center position of compass
#define NEEDLE_L        140 / 2           // 84/2  // Needle length is 84, we want radius which is 42
#define NEEDLE_W        16 / 2            // 12/2  // Needle width is 12, radius is then 6
#define WAIT            20                // Pause in milliseconds to set refresh speed

void drawCompass(int x, int y, int angle);
void getCoord(int x, int y, int *xp, int *yp, int r, int a);

int number = 0;
int angle  = 0;

int lx1 = 0;
int ly1 = 0;
int lx2 = 0;
int ly2 = 0;
int lx3 = 0;
int ly3 = 0;
int lx4 = 0;
int ly4 = 0;

// Test only
uint16_t n  = 0;
uint32_t dt = 0;

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void init_Compass(void)
{
    spr[0].setRotation(0);
    spr[0].fillScreen(BACKGROUND);
    spr[1].setRotation(0);
    spr[1].fillScreen(BACKGROUND);

    tft.startWrite(); // TFT chip select held low permanently

    startMillis = millis();
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
void loop_Compass()
{
    sprPtr[0] = (uint16_t *)spr[0].createSprite(SPRITE_WIDTH, SPRITE_HEIGTH);
    //sprPtr[1] = (uint16_t *)spr[0].createSprite(SPRITE_WIDTH, SPRITE_HEIGTH);
    while (1) {
        drawCompass(X0, Y0, angle); // Draw centre of compass at 50,50
        angle += 3;                 // Increment angle for testing
        if (angle > 359) angle = 0; // Limit angle to 360
        delay(WAIT);
        drawCompass(X0, Y0, angle); // Draw centre of compass at 50,50
        angle += 3;                 // Increment angle for testing
        if (angle > 359) angle = 0; // Limit angle to 360
        delay(WAIT);
    }
    // Delete sprite to free up the RAM
    spr[0].deleteSprite();
    spr[1].deleteSprite();
}

// Test code to measure runtimes, executes code 100x and shows time taken
#define TSTART // n=100;dt=millis();while(n--){
#define TPRINT //};Serial.println((millis()-dt)/100.0);

// #########################################################################
// Draw compass using the defined transparent colour (takes ~6ms)
// #########################################################################
void drawCompass(int x, int y, int angle)
{
    TSTART
    // TFT_TRANSPARENT is a special colour with reversible 8/16 bit coding
    // this allows it to be used in both 8 and 16 bit colour sprites.
    spr[0].fillSprite(TFT_TRANSPARENT);

    // Draw the old needle position in the screen background colour so
    // it gets erased on the TFT when the sprite is drawn
    spr[0].fillTriangle(lx1, ly1, lx3, ly3, lx4, ly4, BACKGROUND);
    spr[0].fillTriangle(lx2, ly2, lx3, ly3, lx4, ly4, BACKGROUND);

    // Set text coordinate datum to middle centre
    spr[0].setTextDatum(MC_DATUM);
    spr[0].setTextColor(TFT_WHITE);

    spr[0].drawString("N", X0, Y0 - NEEDLE_L, 2);
    spr[0].drawString("E", X0 + NEEDLE_L, 50, 2);
    spr[0].drawString("S", X0, Y0 + NEEDLE_L, 2);
    spr[0].drawString("W", X0 - NEEDLE_L, Y0, 2);

    spr[0].drawCircle(X0, Y0, 60, TFT_DARKGREY);

    getCoord(x, y, &lx1, &ly1, NEEDLE_L, angle);
    getCoord(x, y, &lx2, &ly2, NEEDLE_L, angle + 180);
    getCoord(x, y, &lx3, &ly3, NEEDLE_W, angle + 90);
    getCoord(x, y, &lx4, &ly4, NEEDLE_W, angle - 90);

    spr[0].fillTriangle(lx1, ly1, lx3, ly3, lx4, ly4, TFT_RED);
    spr[0].fillTriangle(lx2, ly2, lx3, ly3, lx4, ly4, TFT_LIGHTGREY);

    spr[0].fillCircle(X0, Y0, 3, TFT_DARKGREY);
    spr[0].fillCircle(Y0, X0, 2, TFT_LIGHTGREY);

    // spr[0].pushSprite(SPRITE_CENTER_X - SPRITE_WIDTH / 2, SPRITE_CENTER_Y - SPRITE_HEIGTH / 2, TFT_TRANSPARENT);
    tft.pushImageDMA(SPRITE_CENTER_X - SPRITE_WIDTH / 2, SPRITE_CENTER_Y - SPRITE_HEIGTH / 2, SPRITE_WIDTH, SPRITE_HEIGTH, sprPtr[0]);

    TPRINT
}

#define RAD2DEG 0.0174532925

// Get coordinates of end of a vector, centre at x,y, length r, angle a
// Coordinates are returned to caller via the xp and yp pointers
void getCoord(int x, int y, int *xp, int *yp, int r, int a)
{
    float sx1 = cos((a - 90) * RAD2DEG);
    float sy1 = sin((a - 90) * RAD2DEG);
    *xp       = sx1 * r + x;
    *yp       = sy1 * r + y;
}
