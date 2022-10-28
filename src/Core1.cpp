#if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)

#include <Arduino.h>
#include "Core1.h"
#include "pico/stdlib.h"
#include "TFT.h"
#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
#include "Compass.h"

void checkDataCore0();

void core1_init()
{
    tft_init();
    BouncingCircles::initRandom(); // random seems not to work on core1
}

void core1_loop()
{
    uint32_t demoMillis  = millis();
    uint8_t attitudeType = 1;
/*
    BouncingCircles::init();
    AttitudeIndicator::init(attitudeType);
    Compass::init();
*/
    while (1) {
        demoMillis = millis();
        BouncingCircles::init();
        do {
            BouncingCircles::loop();
            checkDataCore0();
        } while (millis() - demoMillis < 5000);
        BouncingCircles::stop();

        demoMillis = millis();
        AttitudeIndicator::init(2);
        do {
            AttitudeIndicator::loop(1);
            checkDataCore0();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();

        demoMillis = millis();
        Compass::init();
        do {
            Compass::loop();
            checkDataCore0();
        } while (millis() - demoMillis < 5000);
        Compass::stop();

        demoMillis = millis();
        AttitudeIndicator::init(1);
        do {
            AttitudeIndicator::loop(1);
            checkDataCore0();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();
    }
}

void checkDataCore0()
{
    uint16_t loopCounter = 0;
    uint32_t startMillis = millis();
    uint16_t interval    = 10;
    String  fps          = "xx.xx fps";

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
#endif // #if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)