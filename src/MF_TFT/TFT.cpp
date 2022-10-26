#include <Arduino.h>
#include "TFT.h"

int32_t  startMillis = 0;

// Library instance
TFT_eSPI tft = TFT_eSPI();

// Create two sprites for a DMA toggle buffer
TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};

// Pointers to start of Sprites in RAM (these are then "image" pointers)
uint16_t *sprPtr[2];

void tft_init()
{
// #########################################################################
//  reduce systemfrequency to 125MHz to get max SPI speed for 62.5 MHz
//  which is the maximum for most displays.
//  The SPI clock rate can only be set to an integer division of the processor clock.
//  The library will drop the clock to the next lower nearest SPI frequency.
//  So, when the processor frequency is increased to say 133MHz, 
//  then the maximum SPI rate is now 66500000. This means if you specify 62500000
//  as the frequency then the library will drop the rate to the next lowest value of 33250000,
//  so you will see a speed drop.
//  REMERK!! Changing to 125MHz or using PIO for SPI disables servo functionality
//  as the servo implementation uses also the PIO
//  if servo is not needed, uncomment the following function
//  this will speed up SPI transfer
//  For this example 24.7fps will be increased to 40fps
//  Using PIO for SPI will additionally free up the MISO pin
//  running on 133MHz gives 26.2fps, running on 125MHz gives 40.3fps
// #########################################################################
    //set_sys_clock_khz(125000, false);

    // Changing SPI frequency to clk_sys/2
    // clk_peri does not have a divider, so in and out frequencies must be the same
    clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, clock_get_hz(clk_sys), clock_get_hz(clk_sys));

    tft.init();
    tft.initDMA();
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(0);

    randomSeed(analogRead(A0));
    
}