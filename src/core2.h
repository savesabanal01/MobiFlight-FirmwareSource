#pragma once


#define CORE1_CMD        0x80000000
#define CORE1_CMD_STOP   (CORE1_CMD || 0x01000000)
#define CORE1_DATA       0x40000000

void loop_core2();
void init_TFT();
