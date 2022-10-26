#include <Arduino.h>
#include "Core1.h"
#include "pico/stdlib.h"
#include "TFT.h"
#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
#include "Compass.h"

//#define BOUNCING_CIRCLES
#define ATTITUDE_INDICATOR
//#define COMPASS

void core1_init()
{
    tft_init();

#ifdef BOUNCING_CIRCLES
    init_bouncingCircles();
#endif
#ifdef ATTITUDE_INDICATOR
    init_AttitudeIndicator();
#endif
#ifdef COMPASS
    init_Compass();
#endif
}

void core1_loop()
{
    uint32_t millisStart = millis();
    uint8_t  loopCounter = 0;

    while (1) {

#ifdef BOUNCING_CIRCLES
        loop_bouncingCircles();
#endif
#ifdef ATTITUDE_INDICATOR
        // testRoll();
        // testPitch();
        loop_AttitudeIndicator();
#endif
#ifdef COMPASS
        loop_Compass();
#endif

        loopCounter++;
        if (loopCounter == 10) {
            Serial.print("Time for 1 loops: "); Serial.print((millis() - millisStart) / 10); Serial.println("ms");
            millisStart = millis();
            loopCounter = 0;
        }

        // #########################################################################
        // Communication with Core0
        // see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
        // #########################################################################
        if (multicore_fifo_rvalid()) {
            uint32_t dataCore0 = multicore_fifo_pop_blocking();
            if (dataCore0 == CORE1_CMD_STOP) multicore_lockout_victim_init();
            if (dataCore0 & CORE1_DATA) {
                uint32_t receivedData = dataCore0 & 0x00FFFFFF;
            }
        }
    }
}
