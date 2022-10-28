#if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)

#include <Arduino.h>
#include "Core1.h"
#include "pico/stdlib.h"
#include "TFT.h"
#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
#include "Compass.h"

//#define BOUNCING_CIRCLES
//#define ATTITUDE_INDICATOR
#define COMPASS

void core1_init()
{
    tft_init();
    BouncingCircles::initRandom(); // random seems not to work on core1
}

void core1_loop()
{
    uint32_t startMillis  = millis();
    uint16_t interval     = 10;
    uint16_t loopCounter  = 0;
    uint8_t  attitudeType = 1;
    String   fps          = "xx.xx fps";

#ifdef BOUNCING_CIRCLES
    BouncingCircles::init();
#endif
#ifdef ATTITUDE_INDICATOR
    AttitudeIndicator::init(attitudeType);
#endif
#ifdef COMPASS
    Compass::init();
#endif

    while (1) {
#ifdef BOUNCING_CIRCLES
        BouncingCircles::loop();
#endif
#ifdef ATTITUDE_INDICATOR
        AttitudeIndicator::loop(attitudeType);
#endif
#ifdef COMPASS
        Compass::loop();
#endif

        loopCounter++;
        if (loopCounter % interval == 0) {
            long millisSinceUpdate = millis() - startMillis;
            fps                    = String((interval * 1000.0 / (millisSinceUpdate))) + " fps";
Serial.println(fps);
            startMillis = millis();
        }

        // #########################################################################
        // Communication with Core0
        // see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
        // #########################################################################
        if (multicore_fifo_rvalid()) {
            uint32_t dataCore0 = multicore_fifo_pop_blocking();
            // check if bit 32 is set to 1
            if (dataCore0 & CORE1_CMD) {
                if (dataCore0 == CORE1_CMD_STOP) multicore_lockout_victim_init();
            }
            // check if bit 32 is set to 0
            if (!(dataCore0 & CORE1_DATA)) {
                uint32_t receivedData = dataCore0 & 0x00FFFFFF;
                // Do something with your data
            }
        }
    }
}

#endif // #if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)