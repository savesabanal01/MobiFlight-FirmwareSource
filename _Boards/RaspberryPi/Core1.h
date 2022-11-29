#pragma once

// #########################################################################
// For Communication with Core1
// see https://raspberrypi.github.io/pico-sdk-doxygen/group__multicore__fifo.html
// #########################################################################

#define CORE1_CMD        (1 << 31)
#define CORE1_DATA       !(1 << 31)
#define CORE1_CMD_STOP   (CORE1_CMD || (1 << 0))


void core1_init();
void core1_loop();

