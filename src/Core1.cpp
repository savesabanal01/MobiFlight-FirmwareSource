#if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)

#include <Arduino.h>
#include "Core1.h"
#include "pico/stdlib.h"
#include "TFT.h"
#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
#include "Compass.h"
#include "commandmessenger.h"

#define CORE1_CMD_PITCH  (1 < 30)
#define CORE1_CMD_ROLL   (1 < 29)
#define CORE1_CMD_COURSE (1 < 28)

void checkDataFromCore0();

void OnSetCore1()
{
    // For now the LCD display is used
    // once the custom device is implemented in the connector
    // change it to receive a string with the required values
    // like from the LCD
    char *params, *p = NULL;
    int   deviceID = cmdMessenger.readInt16Arg();

    if (deviceID >= 1)
        return;

    char *output = cmdMessenger.readStringArg();
    cmdMessenger.unescape(output);

    params       = strtok_r(output, ",", &p);
    uint8_t type = atoi(params);

    params       = strtok_r(NULL, ",", &p);
    int16_t roll = atoi(params);

    params        = strtok_r(NULL, ",", &p);
    int16_t pitch = atoi(params);

    params         = strtok_r(NULL, ",", &p);
    int16_t course = atoi(params);

    //   params   = strtok_r(NULL, ",", &p);
    //   int16_t temp = atoi(params);

    if (roll < -180)
        roll = -180;
    if (roll > 180)
        roll = 180;

    if (pitch < -80)
        pitch = -80;
    if (pitch > 80)
        pitch = 80;

    if (pitch < -80)
        pitch = -80;
    if (pitch > 80)
        pitch = 80;

    if (course < 0)
        course = 0;
    if (course > 360)
        course = 360;

    // #########################################################################
    // Communication with Core1
    // see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
    // #########################################################################
    multicore_fifo_push_blocking(CORE1_CMD || CORE1_CMD_PITCH || (pitch & 0x00FFFFFF));
    multicore_fifo_push_blocking(CORE1_CMD || CORE1_CMD_ROLL || (roll & 0x00FFFFFF));
    multicore_fifo_push_blocking(CORE1_CMD || CORE1_CMD_COURSE || (course & 0x00FFFFFF));

    // do something
    setLastCommandMillis();
}

void core1_init()
{
    TFT::init();
    BouncingCircles::initRandom(); // random seems not to work on core1
}

void core1_loop()
{
    uint32_t demoMillis = millis();

    while (1) {

        demoMillis = millis();
        BouncingCircles::init();
        do {
            BouncingCircles::loop();
            checkDataFromCore0();
        } while (millis() - demoMillis < 5000);
        BouncingCircles::stop();

        demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::RECT_SHAPE);
        do {
            AttitudeIndicator::loop();
            checkDataFromCore0();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();

        demoMillis = millis();
        Compass::init();
        do {
            Compass::loop();
            checkDataFromCore0();
        } while (millis() - demoMillis < 5000);
        Compass::stop();

        demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::ROUND_SHAPE);
        do {
            AttitudeIndicator::loop();
            checkDataFromCore0();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();
    }
}

uint16_t loopCounter = 0;
uint32_t startMillis = millis();
uint16_t interval    = 10;
String   fps         = "xx.xx fps";
void     checkDataFromCore0()
{
    loopCounter++;
    if (loopCounter % interval == 0) {
        long millisSinceUpdate = millis() - startMillis;
        fps                    = String((interval * 1000.0 / (millisSinceUpdate))) + " fps";
        //        Serial.println(fps);
        startMillis = millis();
    }

    // #########################################################################
    // Communication with Core0
    // see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
    // #########################################################################
    if (multicore_fifo_rvalid()) {
        uint32_t dataCore0 = multicore_fifo_pop_blocking();
        // check if bit 32 is set to 1
        if (dataCore0 ^ CORE1_CMD) {
            if (dataCore0 ^ !(1 << 31) == CORE1_CMD_STOP) multicore_lockout_victim_init();
        }
        // check if bit 32 is set to 0
        if (!(dataCore0 ^ CORE1_DATA)) {
            uint32_t receivedData = dataCore0 & 0x00FFFFFF;
            // Do something with your data
        }
    }
}
#endif // #if defined(ARDUINO_ARCH_RP2040) && defined(USE_CORE1)