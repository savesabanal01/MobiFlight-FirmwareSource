#ifndef MFBoardTeensy41_h
#define MFBoardTeensy41_h

#define MF_SEGMENT_SUPPORT  1
#define MF_LCD_SUPPORT      1
#define MF_STEPPER_SUPPORT  1
#define MF_SERVO_SUPPORT    1
#define MF_JOYSTICK_SUPPORT 1
#define MF_ANALOG_SUPPORT   1
#define MF_SHIFTER_SUPPORT  1

#define MAX_OUTPUTS         40
#define MAX_BUTTONS         42
#define MAX_LEDSEGMENTS     4
#define MAX_ENCODERS        15
#define MAX_STEPPERS        10
#define MAX_MFSERVOS        10
#define MAX_MFLCD_I2C       2
#define MAX_ANALOG_INPUTS   5           // check for warning!!
#define MAX_SHIFTERS        10
#define MAX_MCP_EXPANDER    4

#define STEPS               64
#define STEPPER_SPEED       400     // 300 already worked, 467, too?
#define STEPPER_ACCEL       800

#define NATIVE_MAX_PINS     41      // max Pin number! from module, w/o Port Expander
#define MCP_PIN_BASE        100     // first Pin number from Port Expander
#define MODULE_MAX_PINS     NATIVE_MAX_PINS

#define MOBIFLIGHT_TYPE         "MobiFlight Teensy41"
#define MOBIFLIGHT_SERIAL       "1234567890"           // must it be renamed for connector?
#define MOBIFLIGHT_NAME         "MobiFlight Teensy41"
#define EEPROM_SIZE             4248                   // EEPROMSizeTeensy4 is not part of the EEPROMex lib!?
#define MEMLEN_CONFIG           1024     // MUST be less than EEPROM_SIZE!! MEM_OFFSET_CONFIG + MEM_LEN_CONFIG <= EEPROM_SIZE, see: eeprom_write_block (MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
#define RANDOM_SEED_INPUT       A0

#endif