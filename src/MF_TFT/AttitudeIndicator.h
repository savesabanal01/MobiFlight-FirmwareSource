#pragma once


// Define the width and height according to the TFT and the
// available memory. The sprites will require:
//     DWIDTH * DHEIGHT * 2 bytes of RAM
// Note: for a 240 * 320 area this is 150 Kbytes!
#define DWIDTH  240
#define DHEIGHT 240


void init_AttitudeIndicator();
void loop_AttitudeIndicator();

