#include <Arduino.h>
#include "Wire.h"
#include "SPI.h"
#include "TFT.h"
//#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
//#include "Compass.h"

#include "pin_config.h"
#include "XL9535_driver.h"

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    -1, -1, -1, EXAMPLE_PIN_NUM_DE, EXAMPLE_PIN_NUM_VSYNC, EXAMPLE_PIN_NUM_HSYNC, EXAMPLE_PIN_NUM_PCLK,
    EXAMPLE_PIN_NUM_DATA1, EXAMPLE_PIN_NUM_DATA2, EXAMPLE_PIN_NUM_DATA3, EXAMPLE_PIN_NUM_DATA4, EXAMPLE_PIN_NUM_DATA5,
    EXAMPLE_PIN_NUM_DATA6, EXAMPLE_PIN_NUM_DATA7, EXAMPLE_PIN_NUM_DATA8, EXAMPLE_PIN_NUM_DATA9, EXAMPLE_PIN_NUM_DATA10, EXAMPLE_PIN_NUM_DATA11,
    EXAMPLE_PIN_NUM_DATA13, EXAMPLE_PIN_NUM_DATA14, EXAMPLE_PIN_NUM_DATA15, EXAMPLE_PIN_NUM_DATA16, EXAMPLE_PIN_NUM_DATA17);
Arduino_GFX *gfx = new Arduino_ST7701_RGBPanel(bus, GFX_NOT_DEFINED, 0 /* rotation */, false /* IPS */, 480, 480,
                                               st7701_type2_init_operations, sizeof(st7701_type2_init_operations), true,
                                               50, 1, 30, 20, 1, 30);

XL9535 xl;

namespace TFT
{

    void tft_init(void);
    void lcd_cmd(const uint8_t cmd);
    void lcd_data(const uint8_t *data, int len);

    typedef struct {
        uint8_t cmd;
        uint8_t data[16];
        uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
    } lcd_init_cmd_t;

    DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[] = {
        {0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 0x05},
        {0xC0, {0x3b, 0x00}, 0x02},
        {0xC1, {0x0b, 0x02}, 0x02},
        {0xC2, {0x07, 0x02}, 0x02},
        {0xCC, {0x10}, 0x01},
        {0xCD, {0x08}, 0x01}, // 用565时屏蔽    666打开
        {0xb0, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x06, 0x05, 0x09, 0x08, 0x21, 0x06, 0x13, 0x10, 0x29, 0x31, 0x18}, 0x10},
        {0xb1, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x07, 0x05, 0x09, 0x09, 0x21, 0x05, 0x13, 0x11, 0x2a, 0x31, 0x18}, 0x10},
        {0xFF, {0x77, 0x01, 0x00, 0x00, 0x11}, 0x05},
        {0xb0, {0x6d}, 0x01},
        {0xb1, {0x37}, 0x01},
        {0xb2, {0x81}, 0x01},
        {0xb3, {0x80}, 0x01},
        {0xb5, {0x43}, 0x01},
        {0xb7, {0x85}, 0x01},
        {0xb8, {0x20}, 0x01},
        {0xc1, {0x78}, 0x01},
        {0xc2, {0x78}, 0x01},
        {0xc3, {0x8c}, 0x01},
        {0xd0, {0x88}, 0x01},
        {0xe0, {0x00, 0x00, 0x02}, 0x03},
        {0xe1, {0x03, 0xa0, 0x00, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00, 0x20, 0x20}, 0x0b},
        {0xe2, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x0d},
        {0xe3, {0x00, 0x00, 0x11, 0x00}, 0x04},
        {0xe4, {0x22, 0x00}, 0x02},
        {0xe5, {0x05, 0xec, 0xa0, 0xa0, 0x07, 0xee, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
        {0xe6, {0x00, 0x00, 0x11, 0x00}, 0x04},
        {0xe7, {0x22, 0x00}, 0x02},
        {0xe8, {0x06, 0xed, 0xa0, 0xa0, 0x08, 0xef, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
        {0xeb, {0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x07},
        {0xed, {0xff, 0xff, 0xff, 0xba, 0x0a, 0xbf, 0x45, 0xff, 0xff, 0x54, 0xfb, 0xa0, 0xab, 0xff, 0xff, 0xff}, 0x10},
        {0xef, {0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f}, 0x06},
        {0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 0x05},
        {0xef, {0x08}, 0x01},
        {0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 0x05},
        {0x36, {0x08}, 0x01},
        {0x3a, {0x66}, 0x01},
        {0x11, {0x00}, 0x80},
        // {0xFF, {0x77, 0x01, 0x00, 0x00, 0x12}, 0x05},
        // {0xd1, {0x81}, 0x01},
        // {0xd2, {0x06}, 0x01},
        {0x29, {0x00}, 0x80},
        {0, {0}, 0xff}};

    int16_t checkClippingRoundOuter[MAX_CLIPPING_RADIUS] = {0};
    int16_t checkClippingRoundInner[MAX_CLIPPING_RADIUS] = {0};

    int16_t clippingCenterX;
    int16_t clippingCenterY;
    int16_t clippingWidthX;
    int16_t clippingWidthY;
    int16_t clippingRadiusOuter;
    int16_t clippingRadiusInner;

    void init()
    {
        Wire.begin(IIC_SDA_PIN, IIC_SCL_PIN, (uint32_t)800000);
        xl.begin();
        uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                      (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);
        xl.pinMode8(0, pin, OUTPUT);
        xl.digitalWrite(PWR_EN_PIN, 1);
        pinMode(EXAMPLE_PIN_NUM_BK_LIGHT, OUTPUT);
        digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
        gfx->begin();
        tft_init();
        gfx->fillScreen(BLACK);
        /*
        gfx->setCursor(100, 240);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(3);
        gfx->println("Mobiflight rocks!");
        */
        delay(2000);

        uint32_t demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::ROUND_SHAPE);
        do {
            AttitudeIndicator::loop();
            // checkDataFromCore0();
        } while (millis() - demoMillis < 10000000);
    }

    // setup clipping area
    void setClippingArea(int16_t ClippingX0, int16_t ClippingY0, int16_t ClippingXwidth, int16_t ClippingYwidth, int16_t ClippingRadiusOuter, int16_t ClippingRadiusInner)
    {
        clippingCenterX     = ClippingX0;
        clippingCenterY     = ClippingY0;
        clippingWidthX      = ClippingXwidth;
        clippingWidthY      = ClippingYwidth;
        clippingRadiusOuter = ClippingRadiusOuter;
        clippingRadiusInner = ClippingRadiusInner;

        if (checkClippingRoundOuter[0] != ClippingRadiusOuter) {
            checkClippingRoundOuter[0] = clippingRadiusOuter;
            for (uint8_t i = 1; i < clippingRadiusOuter; i++) {
                checkClippingRoundOuter[i] = sqrt(clippingRadiusOuter * clippingRadiusOuter - i * i);
            }
        }
        if (checkClippingRoundInner[0] != ClippingRadiusInner) {
            checkClippingRoundInner[0] = clippingRadiusInner;
            for (uint8_t i = 1; i < clippingRadiusInner; i++) {
                checkClippingRoundInner[i] = sqrt(clippingRadiusInner * clippingRadiusInner - i * i);
            }
        }
    }

    // #########################################################################
    // Helper functions transferred from the lib for a round clipping area
    // Before using this functions refresh rate was 21ms
    // Without Clipping it mostly 21ms, sometimes 70ms
    // With rectangular clipping it is still 21ms with sometimes 70ms
    // With round clipping it is 21 - 22ms
    // #########################################################################

    /***************************************************************************************
    ** Function name:           drawLineClipped
    ** Description:             draw a line between 2 arbitrary points
    ***************************************************************************************/
    // Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
    // an efficient FastH/V Line draw routine for line segments of 2 pixels or more
    void drawLineClipped(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, bool sel)
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

        int16_t dx = x1 - x0, dy = abs(y1 - y0);

        int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

        if (y0 < y1) ystep = 1;

        // Split into steep and not steep for FastH/V separation
        if (steep) {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawLineClipped(y0, xs, color, sel);
                    else
                        drawFastVLineClipped(y0, xs, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
                    err += dx;
                }
            }
            if (dlen) drawFastVLineClipped(y0, xs, dlen, color, sel);
        } else {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawLineClipped(xs, y0, color, sel);
                    else
                        drawFastHLineClipped(xs, y0, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
                    err += dx;
                }
            }
            if (dlen) drawFastHLineClipped(xs, y0, dlen, color, sel);
        }
    }

    /***************************************************************************************
    ** Function name:           drawFastHLineClipped
    ** Description:             draw a horizontal line
    ***************************************************************************************/
    void drawFastHLineClipped(int16_t x, int16_t y, int16_t w, uint16_t color, bool sel)
    {
        // draw always from left to right
        if (w < 0) {
            x -= w;
            w *= -1;
        }
        int16_t xE = x + w;

        if (clippingRadiusOuter == 0) {
            // First check upper and lower limits, it's quite easy
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
            // check left and right limit and set start / end point accordingly
            // ToDo so:
            // first check if end of line is outside clipping area
            if (xE <= clippingCenterX - clippingWidthX / 2)
                return;
            // next check if end of line is outside clipping area
            if (x >= clippingCenterX + clippingWidthX / 2)
                return;
            // next check if only start of line is out of clipping area
            if (x <= clippingCenterX - clippingWidthX / 2) x = clippingCenterX - clippingWidthX / 2 + 1;
            // and lest check if only end of line is out of clipping area
            if (xE >= clippingCenterX + clippingWidthX / 2) xE = clippingCenterX + clippingWidthX / 2 - 1;
        } else {
            // First check upper and lower limits, it's quite easy
            if (y <= clippingCenterY - clippingRadiusOuter || y >= clippingCenterY + clippingRadiusOuter) return;
            // check left and right limit and set start / end point accordingly from look up table for the given x position
            // ToDo so:
            // first check if end of line is outside clipping area
            if (xE <= clippingCenterX - checkClippingRoundOuter[abs(y - clippingCenterY)])
                return;
            // next check if end of line is outside clipping area
            if (x >= clippingCenterX + checkClippingRoundOuter[abs(y - clippingCenterY)])
                return;
            // next check if only start of line is out of clipping area
            if (x <= clippingCenterX - checkClippingRoundOuter[abs(y - clippingCenterY)]) x = clippingCenterX - checkClippingRoundOuter[abs(y - clippingCenterY)];
            // and lest check if only end of line is out of clipping area
            if (xE >= clippingCenterX + checkClippingRoundOuter[abs(y - clippingCenterY)]) xE = clippingCenterX + checkClippingRoundOuter[abs(y - clippingCenterY)];
        }
        if (clippingRadiusInner > 0) {
            // at this point we have already the x/y coordinates for the outer circle
            // now calculate the x/y coordinates for the inner circle to split into two lines or for "big" y-values still in one line
            // this should be the case if y is bigger or smaller than the inner radius
            if (y <= clippingCenterY - clippingRadiusInner || y >= clippingCenterY + clippingRadiusInner) {
                x  = clippingCenterX - checkClippingRoundOuter[abs(y - clippingCenterY)];
                xE = clippingCenterX + checkClippingRoundOuter[abs(y - clippingCenterY)];
            } else {
                // now calculate the x/y coordinates for the inner circle to split into two lines
                // y coordinate is known, nothing to change
                // x axis must be split up according the inner radius
                int16_t tempxA = clippingCenterX - checkClippingRoundOuter[abs(y - clippingCenterY)];
                int16_t tempxE = clippingCenterX - checkClippingRoundInner[abs(y - clippingCenterY)];
                // draw the left short line
                gfx->writeFastHLine(tempxA, y, tempxE - x + 1, color);
                // and calculate the coordinates for the right short line
                x = clippingCenterX + checkClippingRoundInner[abs(y - clippingCenterY)];
            }
        }
        gfx->writeFastHLine(x, y, xE - x + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawFastVLineClipped
    ** Description:             draw a vertical line
    ***************************************************************************************/
    void drawFastVLineClipped(int16_t x, int16_t y, int16_t h, uint16_t color, bool sel)
    {
        // draw always from top to down
        if (h < 0) {
            y -= h;
            h *= -1;
        }
        int16_t yE = y + h;
        if (clippingRadiusOuter == 0) {
            // First check left and right limits, it's quite easy
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
            // check upper and lower limit and set start / end point accordingly
            // ToDo so:
            // first check if end of line is outside clipping area
            if (yE <= clippingCenterY - clippingWidthY / 2)
                return;
            // next check if end of line is outside clipping area
            if (y >= clippingCenterY + clippingWidthY / 2)
                return;
            // next check if only start of line is out of clipping area
            if (y <= clippingCenterY - clippingWidthY / 2) y = clippingCenterY - clippingWidthY / 2 + 1;
            // and lest check if only end of line is out of clipping area
            if (yE >= clippingCenterY + clippingWidthY / 2) yE = clippingCenterY + clippingWidthY / 2 - 1;
        } else {
            // First check left and right limits, it's quite easy
            if (x <= clippingCenterX - clippingRadiusOuter || x >= clippingCenterX + clippingRadiusOuter) return;
            // check upper and lower limit and set start / end point accordingly from look up table for the given x position
            // ToDo so:
            // first check if end of line is outside clipping area
            if (yE <= clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)])
                return;
            // next check if end of line is outside clipping area
            if (y >= clippingCenterY + checkClippingRoundOuter[abs(x - clippingCenterX)])
                return;
            // next check if only start of line is out of clipping area
            if (y <= clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)]) y = clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)];
            // and lest check if only end of line is out of clipping area
            if (yE >= clippingCenterY + checkClippingRoundOuter[abs(x - clippingCenterX)]) yE = clippingCenterY + checkClippingRoundOuter[abs(x - clippingCenterX)];
        }
        if (clippingRadiusInner > 0) {
            // at this point we have already the x/y coordinates for the outer circle
            // now calculate the x/y coordinates for the inner circle to split into two lines or for "big" x-values still in one line
            // this should be the case if x is bigger or smaller than the inner radius
            if (x <= clippingCenterX - clippingRadiusOuter || x >= clippingCenterX + clippingRadiusOuter) {
                y  = clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)];
                yE = clippingCenterY + checkClippingRoundOuter[abs(x - clippingCenterX)];
            } else {
                // now calculate the x/y coordinates for the inner circle to split into two lines
                // x coordinate is known, nothing to change
                // y axis must be split up according the inner radius
                int16_t tempyA = clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)];
                int16_t tempyE = clippingCenterY - checkClippingRoundInner[abs(x - clippingCenterX)];
                // draw the upper short line
                gfx->writeFastVLine(x, y, tempyE - x + 1, color);
                // and calculate the coordinates for the lower short line
                y = clippingCenterY + checkClippingRoundInner[abs(x - clippingCenterX)];
            }
        }
        gfx->writeFastVLine(x, y, yE - y + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawLineClipped
    ** Description:             push a single pixel at an arbitrary position
    ***************************************************************************************/
    void drawLineClipped(int16_t x, int16_t y, uint16_t color, bool sel)
    {
        if (clippingRadiusOuter == 0) {
            // for a rect clipping area just check upper/lower and left/right limit
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
        } else {
            // First do a rect clipping
            if (x <= clippingCenterX - clippingRadiusOuter || x >= clippingCenterX + clippingRadiusOuter) return;
            if (y <= clippingCenterY - clippingRadiusOuter || y >= clippingCenterY + clippingRadiusOuter) return;
            // next check if Pixel is within circel or outside
            if (y < clippingCenterY - checkClippingRoundOuter[abs(x - clippingCenterX)]) return;
            if (y > clippingCenterY + checkClippingRoundOuter[abs(x - clippingCenterX)]) return;
        }
        if (clippingRadiusInner > 0) {
            // First do a rect clipping, check if we are INSIDE both radi
            if (!(x <= clippingCenterX - clippingRadiusInner || x >= clippingCenterX + clippingRadiusInner)) return;
            if (!(y <= clippingCenterY - clippingRadiusInner || y >= clippingCenterY + clippingRadiusInner)) return;
            // next check if Pixel is within circel or outside
            if (!(y < clippingCenterY - checkClippingRoundInner[abs(x - clippingCenterX)])) return;
            if (!(y > clippingCenterY + checkClippingRoundInner[abs(x - clippingCenterX)])) return;
        }
        gfx->drawPixel(x, y, color);
    }

    /***************************************************************************************
     ** Function name:           fillHalfCircle
     ** Description:             draw a filled circle, upper or lower part
     ***************************************************************************************/
    // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
    // Improved algorithm avoids repetition of lines
    void fillHalfCircleTFT(int16_t x0, int16_t y0, int16_t r, uint16_t colorUpper, uint16_t colorLower)
    {
        int16_t x  = 0;
        int16_t dx = 1;
        int16_t dy = r + r;
        int16_t p  = -(r >> 1);

        gfx->writeFastHLine(x0 - r, y0, dy + 1, colorUpper);

        while (x < r) {

            if (p >= 0) {
                gfx->writeFastHLine(x0 - x, y0 - r, dx, colorUpper);
                gfx->writeFastHLine(x0 - x, y0 + r, dx, colorLower);
                dy -= 2;
                p -= dy;
                r--;
            }

            dx += 2;
            p += dx;
            x++;

            gfx->writeFastHLine(x0 - r, y0 - x, dy + 1, colorUpper);
            gfx->writeFastHLine(x0 - r, y0 + x, dy + 1, colorLower);
        }
    }

    void tft_init(void)
    {
        xl.digitalWrite(LCD_CS_PIN, 1);
        xl.digitalWrite(LCD_SDA_PIN, 1);
        xl.digitalWrite(LCD_CLK_PIN, 1);

        // Reset the display
        xl.digitalWrite(LCD_RST_PIN, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        xl.digitalWrite(LCD_RST_PIN, 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        xl.digitalWrite(LCD_RST_PIN, 1);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        int cmd = 0;
        while (st_init_cmds[cmd].databytes != 0xff) {
            lcd_cmd(st_init_cmds[cmd].cmd);
            lcd_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes & 0x1F);
            if (st_init_cmds[cmd].databytes & 0x80) {
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            cmd++;
        }
        Serial.println("Register setup complete");
    }

    void lcd_send_data(uint8_t data)
    {
        uint8_t n;
        for (n = 0; n < 8; n++) {
            if (data & 0x80)
                xl.digitalWrite(LCD_SDA_PIN, 1);
            else
                xl.digitalWrite(LCD_SDA_PIN, 0);

            data <<= 1;
            xl.digitalWrite(LCD_CLK_PIN, 0);
            xl.digitalWrite(LCD_CLK_PIN, 1);
        }
    }

    void lcd_cmd(const uint8_t cmd)
    {
        xl.digitalWrite(LCD_CS_PIN, 0);
        xl.digitalWrite(LCD_SDA_PIN, 0);
        xl.digitalWrite(LCD_CLK_PIN, 0);
        xl.digitalWrite(LCD_CLK_PIN, 1);
        lcd_send_data(cmd);
        xl.digitalWrite(LCD_CS_PIN, 1);
    }

    void lcd_data(const uint8_t *data, int len)
    {
        uint32_t i = 0;
        if (len == 0)
            return; // no need to send anything
        do {
            xl.digitalWrite(LCD_CS_PIN, 0);
            xl.digitalWrite(LCD_SDA_PIN, 1);
            xl.digitalWrite(LCD_CLK_PIN, 0);
            xl.digitalWrite(LCD_CLK_PIN, 1);
            lcd_send_data(*(data + i));
            xl.digitalWrite(LCD_CS_PIN, 1);
            i++;
        } while (len--);
    }

} // end of namespace TFT