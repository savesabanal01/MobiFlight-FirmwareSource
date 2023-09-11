//
// MFBoards.h (Teensy 35)
//
// (C) MobiFlight Project 2022
//

#ifndef MFBoardTeensy35_h
#define MFBoardTeensy35_h

#ifndef MF_SEGMENT_SUPPORT
#define MF_SEGMENT_SUPPORT 1
#endif
#ifndef MF_LCD_SUPPORT
#define MF_LCD_SUPPORT 1
#endif
#ifndef MF_STEPPER_SUPPORT
#define MF_STEPPER_SUPPORT 1
#endif
#ifndef MF_SERVO_SUPPORT
#define MF_SERVO_SUPPORT 1
#endif
#ifndef MF_ANALOG_SUPPORT
#define MF_ANALOG_SUPPORT 1
#endif
#ifndef MF_OUTPUT_SHIFTER_SUPPORT
#define MF_OUTPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_INPUT_SHIFTER_SUPPORT
#define MF_INPUT_SHIFTER_SUPPORT 1
#endif
#ifndef MF_MUX_SUPPORT
#define MF_MUX_SUPPORT 1
#endif
#ifndef MF_DIGIN_MUX_SUPPORT
#define MF_MUX_SUPPORT       1
#define MF_DIGIN_MUX_SUPPORT 1
#endif
#ifndef MF_KEYMATRIX_SUPPORT
#define MF_KEYMATRIX_SUPPORT 1
#endif

#ifndef MAX_OUTPUTS
#define MAX_OUTPUTS 40
#endif
#ifndef MAX_BUTTONS
#define MAX_BUTTONS 56
#endif
#ifndef MAX_LEDSEGMENTS
#define MAX_LEDSEGMENTS 4
#endif
#ifndef MAX_ENCODERS
#define MAX_ENCODERS 15
#endif
#ifndef MAX_STEPPERS
#define MAX_STEPPERS 10
#endif
#ifndef MAX_MFSERVOS
#define MAX_MFSERVOS 10
#endif
#ifndef MAX_MFLCD_I2C
#define MAX_MFLCD_I2C 2
#endif
#ifndef MAX_ANALOG_INPUTS
#define MAX_ANALOG_INPUTS 5 // check for warning!!
#endif
#ifndef MAX_OUTPUT_SHIFTERS
#define MAX_OUTPUT_SHIFTERS 4
#endif
#ifndef MAX_INPUT_SHIFTERS
#define MAX_INPUT_SHIFTERS 4
#endif
#ifndef MAX_DIGIN_MUX
#define MAX_DIGIN_MUX 2
#endif
#ifndef MAX_KEYMATRIX
#define MAX_KEYMATRIX        1
#define MAX_COLUMN_KEYMATRIX 8
#define MAX_ROW_KEYMATRIX    8
#endif

#define NATIVE_MAX_PINS 57  // max Pin number! from module, w/o Port Expander
#define MCP_PIN_BASE    100 // first Pin number from Port Expander
#define MODULE_MAX_PINS NATIVE_MAX_PINS

#ifndef MOBIFLIGHT_TYPE
#define MOBIFLIGHT_TYPE     "MobiFlight Teensy35"
#endif
#ifndef MOBIFLIGHT_SERIAL
#define MOBIFLIGHT_SERIAL   "1234567890"
#endif
#ifndef MOBIFLIGHT_NAME
#define MOBIFLIGHT_NAME     "MobiFlight Teensy35"
#endif
#ifndef EEPROM_SIZE
#define EEPROM_SIZE         4096 // EEPROMSizeTeensy3
#endif
#ifndef MEMLEN_CONFIG
#define MEMLEN_CONFIG       1496 // MUST be less than EEPROM_SIZE!! MEM_OFFSET_CONFIG + MEM_LEN_CONFIG <= EEPROM_SIZE, see: eeprom_write_block (MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
#endif
#ifndef MEMLEN_NAMES_BUFFER
#define MEMLEN_NAMES_BUFFER 1000 // max. size for configBuffer, contains only names from inputs
#endif
#ifndef MF_MAX_DEVICEMEM
#define MF_MAX_DEVICEMEM    1500 // max. memory size for devices
#endif

//#define RANDOM_SEED_INPUT A0

#endif