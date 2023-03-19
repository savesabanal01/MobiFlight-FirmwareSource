#pragma once

#define X0_RECT           0                      // upper left x position where to plot
#define Y0_RECT           20                     // upper left y position where to plot
#define WIDTH_RECT_INNER  200                    // width of inner moving rect instrument
#define HEIGTH_RECT_INNER 240                    // height of inner moving rect instrument
#define WIDTH_RECT_OUTER  240                    // width of outer part of instrument
#define HEIGTH_RECT_OUTER 280                    // height of outer part of instrument
#define CENTER_X0_RECT    WIDTH_RECT_OUTER / 2   // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_RECT    HEIGTH_RECT_OUTER / 2  // y mid point in sprite for instrument, complete drawing must be inside sprite
#define SPRITE_DIM_RADIUS 120                    // dimension for x and y direction of sprite, including outer part
#define X0_ROUND          0                      // upper left x position where to plot
#define Y0_ROUND          40                     // upper left y position where to plot
#define CENTER_X0_ROUND   SPRITE_DIM_RADIUS      // x mid point in sprite for instrument, complete drawing must be inside sprite
#define CENTER_Y0_ROUND   SPRITE_DIM_RADIUS      // y mid point in sprite for instrument, complete drawing must be inside sprite
#define OUTER_RADIUS      SPRITE_DIM_RADIUS      // radius of outer part of instrument
#define INNER_RADIUS      SPRITE_DIM_RADIUS - 21 // radius of moving part of instrument
#define HOR_RECT          350                    // Horizon vector line, length must be at least sqrt(WIDTH_RECT_INNER^2 + HEIGTH_RECT_INNER^2) + MAX_PITCH = 520
#define HOR_ROUND         350                    // Horizon vector line, length must be at least sqrt(2)*INNER_RADIUS + MAX_PITCH = 520
#define MAX_PITCH         100                    // Maximum pitch shouls be in range +/- 80 with HOR = 172, 20 steps = 10 degrees on drawn scale
#define BROWN             0xFD20                 // 0x5140 // 0x5960 the other are not working??
#define SKY_BLUE          0x02B5                 // 0x0318 //0x039B //0x34BF
#define DARK_RED          TFT_RED                // 0x8000
#define DARK_GREY         TFT_BLACK              // ILI9341_DARKGREY
#define LIGHT_GREY        TFT_BLACK              // ILI9341_LIGHTGREY
#define DEG2RAD           0.0174532925

namespace AttitudeIndicator
{
    enum {
        ROUND_SHAPE = 1,
        RECT_SHAPE
    };
    
    void init(uint8_t type);
    void loop();
    void stop();
}
