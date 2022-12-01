#if defined(USE_CORE0)

#include <Arduino.h>
#include "Core0.h"
#include "TFT.h"
//#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
//#include "Compass.h"
#include "commandmessenger.h"

void     calculateFPS();

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

    // do something
    setLastCommandMillis();
}

void core0_init()
{
    TFT::init();
    //BouncingCircles::initRandom(); // random seems not to work on core1
}

void core0_loop(void * parameter)
{
    uint32_t demoMillis = millis();

    while (1) {
/*
        demoMillis = millis();
        BouncingCircles::init();
        do {
            BouncingCircles::loop();
            calculateFPS();
        } while (millis() - demoMillis < 5000);
        BouncingCircles::stop();
*/
        demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::RECT_SHAPE);
        do {
            AttitudeIndicator::loop();
            calculateFPS();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();
/*
        demoMillis = millis();
        Compass::init();
        do {
            Compass::loop();
            calculateFPS();
        } while (millis() - demoMillis < 5000);
        Compass::stop();
*/
        demoMillis = millis();
        AttitudeIndicator::init(AttitudeIndicator::ROUND_SHAPE);
        do {
            AttitudeIndicator::loop();
            calculateFPS();
        } while (millis() - demoMillis < 10000);
        AttitudeIndicator::stop();
    }
}

uint16_t loopCounter = 0;
uint32_t startMillis = millis();
uint16_t interval    = 10;
String   fps         = "xx.xx fps";
void     calculateFPS()
{
    loopCounter++;
    if (loopCounter % interval == 0) {
        long millisSinceUpdate = millis() - startMillis;
        fps                    = String((interval * 1000.0 / (millisSinceUpdate))) + " fps";
        //        Serial.println(fps);
        startMillis = millis();
    }
}
#endif // #if defined(USE_CORE0)