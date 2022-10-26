#include <Arduino.h>
#include "Core1.h"
#include "pico/stdlib.h"
#include "TFT.h"
#include "bouncingCircles.h"
#include "AttitudeIndicator.h"
#include "Compass.h"


void core1_init()
{
    tft_init();
    //init_bouncingCircles();
    //init_AttitudeIndicator();
    init_Compass();
}

void core1_loop()
{
    //loop_bouncingCircles();
    //loop_AttitudeIndicator();
    loop_Compass();

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
