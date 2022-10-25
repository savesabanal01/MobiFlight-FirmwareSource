#include <Arduino.h>
#include "Core1.h"
#include "bouncingCircles.h"


void core1_init() {
    init_bouncingCircles();
}

void core1_loop() {
    while (1) {
        loop_bouncingCircles();

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
