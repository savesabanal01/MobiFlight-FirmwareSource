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
        gfx->setCursor(100, 240);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(3);
        gfx->println("Mobiflight rocks!");
        delay(2000);
        uint32_t demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::ROUND_SHAPE);
        do {
            AttitudeIndicator::loop();
            //checkDataFromCore0();
        } while (millis() - demoMillis < 100000);
    }

    int32_t checkClippingArea[MAX_CLIPPING_RADIUS];

    int32_t clippingCenterX;
    int32_t clippingCenterY;
    int32_t clippingWidthX;
    int32_t clippingWidthY;
    int32_t clippingRadius;

    // setup clipping area
    void setClippingArea(int32_t ClippingX0, int32_t ClippingY0, int32_t ClippingXwidth, int32_t ClippingYwidth, int32_t ClippingRadius)
    {
        checkClippingArea[0] = ClippingRadius;
        for (uint8_t i = 1; i < ClippingRadius; i++) {
            checkClippingArea[i] = sqrt(ClippingRadius * ClippingRadius - i * i);
        }
        clippingCenterX = ClippingX0;
        clippingCenterY = ClippingY0;
        clippingWidthX  = ClippingXwidth;
        clippingWidthY  = ClippingYwidth;
        clippingRadius  = ClippingRadius;
    }

    // #########################################################################
    // Helper functions transferred from the lib for a round clipping area
    // Before using this functions refresh rate was 21ms
    // Without Clipping it mostly 21ms, sometimes 70ms
    // With rectangular clipping it is still 21ms with sometimes 70ms
    // With round clipping it is 21 - 22ms
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

        int32_t dx = x1 - x0, dy = abs(y1 - y0);

        int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

        if (y0 < y1) ystep = 1;

        // Split into steep and not steep for FastH/V separation
        if (steep) {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawPixel(y0, xs, color, sel);
                    else
                        drawFastVLine(y0, xs, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
                    err += dx;
                }
            }
            if (dlen) drawFastVLine(y0, xs, dlen, color, sel);
        } else {
            for (; x0 <= x1; x0++) {
                dlen++;
                err -= dy;
                if (err < 0) {
                    if (dlen == 1)
                        drawPixel(xs, y0, color, sel);
                    else
                        drawFastHLine(xs, y0, dlen, color, sel);
                    dlen = 0;
                    y0 += ystep;
                    xs = x0 + 1;
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
        // First check upper and lower limits, it's quite easy
        if (clippingRadius > 0) {
            if (y <= clippingCenterY - clippingRadius || y >= clippingCenterY + clippingRadius) return;
        } else {
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
        }
        // always draw from left to right
        if (w < 0) {
            x -= w;
            w *= -1;
        }
        int32_t xE = x + w;
        // check if pixel is inside the radius
        if (clippingRadius > 0) {
            // check left and right limit and set start / end point accordingly from look up table for the given x position
            if (x <= clippingCenterX - checkClippingArea[abs(y - clippingCenterY)]) x = clippingCenterX - checkClippingArea[abs(y - clippingCenterY)];
            if (xE >= clippingCenterX + checkClippingArea[abs(y - clippingCenterY)]) xE = clippingCenterX + checkClippingArea[abs(y - clippingCenterY)];
        } else {
            // check left and right limit and set start / end point accordingly
            if (x <= clippingCenterX - clippingWidthX / 2) x = clippingCenterX - clippingWidthX / 2 + 1;
            if (xE >= clippingCenterX + clippingWidthX / 2) xE = clippingCenterX + clippingWidthX / 2 - 1;
        }
        gfx->drawFastHLine(x, y, xE - x + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawFastVLine
    ** Description:             draw a vertical line
    ***************************************************************************************/
    void drawFastVLine(int32_t x, int32_t y, int32_t h, uint32_t color, bool sel)
    {
        if (clippingRadius > 0) {
            if (x <= clippingCenterX - clippingRadius || x >= clippingCenterX + clippingRadius) return;
        } else {
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
        }
        if (h < 0) {
            y -= h;
            h *= -1;
        }
        int32_t yE = y + h;
        if (clippingRadius > 0) {
            // check upper and lower limit and set start / end point accordingly from look up table for the given x position
            if (y <= clippingCenterY - checkClippingArea[abs(x - clippingCenterX)]) y = clippingCenterY - checkClippingArea[abs(x - clippingCenterX)];
            if (yE >= clippingCenterY + checkClippingArea[abs(x - clippingCenterX)]) yE = clippingCenterY + checkClippingArea[abs(x - clippingCenterX)];
        } else {
            // check left and right limit and set start / end point accordingly
            if (y <= clippingCenterY - clippingWidthY / 2) y = clippingCenterY - clippingWidthY / 2 + 1;
            if (yE >= clippingCenterY + clippingWidthY / 2) yE = clippingCenterY + clippingWidthY / 2 - 1;
        }
        gfx->drawFastVLine(x, y, yE - y + 1, color);
    }

    /***************************************************************************************
    ** Function name:           drawPixel
    ** Description:             push a single pixel at an arbitrary position
    ***************************************************************************************/
    void drawPixel(int32_t x, int32_t y, uint32_t color, bool sel)
    {
        if (clippingRadius > 0) {
            // First do a rect clipping
            if (x <= clippingCenterX - clippingRadius || x >= clippingCenterX + clippingRadius) return;
            if (y <= clippingCenterY - clippingRadius || y >= clippingCenterY + clippingRadius) return;
            // next check if Pixel is within circel or outside
            if (y < clippingCenterY - checkClippingArea[abs(x - clippingCenterX)]) return;
            if (y > clippingCenterY + checkClippingArea[abs(x - clippingCenterX)]) return;
        } else {
            // for a rect clipping area just check upper/lower and left/right limit
            if (x <= clippingCenterX - clippingWidthX / 2 || x >= clippingCenterX + clippingWidthX / 2) return;
            if (y <= clippingCenterY - clippingWidthY / 2 || y >= clippingCenterY + clippingWidthY / 2) return;
        }
        gfx->drawPixel(x, y, color);
    }

    /***************************************************************************************
    ** Function name:           fillHalfCircle
    ** Description:             draw a filled circle, upper or lower part
    ***************************************************************************************/
    // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
    // Improved algorithm avoids repetition of lines
    void fillHalfCircleSprite(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower, bool sel)
    {
        int32_t x  = 0;
        int32_t dx = 1;
        int32_t dy = r + r;
        int32_t p  = -(r >> 1);

        gfx->drawFastHLine(x0 - r, y0, dy + 1, colorUpper);

        while (x < r) {

            if (p >= 0) {
                gfx->drawFastHLine(x0 - x, y0 - r, dx, colorUpper);
                gfx->drawFastHLine(x0 - x, y0 + r, dx, colorLower);
                dy -= 2;
                p -= dy;
                r--;
            }

            dx += 2;
            p += dx;
            x++;

            gfx->drawFastHLine(x0 - r, y0 - x, dy + 1, colorUpper);
            gfx->drawFastHLine(x0 - r, y0 + x, dy + 1, colorLower);
        }
    }

    /***************************************************************************************
     ** Function name:           fillHalfCircle
     ** Description:             draw a filled circle, upper or lower part
     ***************************************************************************************/
    // Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
    // Improved algorithm avoids repetition of lines
    void fillHalfCircleTFT(int32_t x0, int32_t y0, int32_t r, uint32_t colorUpper, uint32_t colorLower)
    {
        int32_t x  = 0;
        int32_t dx = 1;
        int32_t dy = r + r;
        int32_t p  = -(r >> 1);

        gfx->drawFastHLine(x0 - r, y0, dy + 1, colorUpper);

        while (x < r) {

            if (p >= 0) {
                gfx->drawFastHLine(x0 - x, y0 - r, dx, colorUpper);
                gfx->drawFastHLine(x0 - x, y0 + r, dx, colorLower);
                dy -= 2;
                p -= dy;
                r--;
            }

            dx += 2;
            p += dx;
            x++;

            gfx->drawFastHLine(x0 - r, y0 - x, dy + 1, colorUpper);
            gfx->drawFastHLine(x0 - r, y0 + x, dy + 1, colorLower);
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