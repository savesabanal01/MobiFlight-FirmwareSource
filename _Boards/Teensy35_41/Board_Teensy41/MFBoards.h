#ifndef MFBoardTeensy41_h
#define MFBoardTeensy41_h

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

#define MAX_OUTPUTS         40
#define MAX_BUTTONS         42
#define MAX_LEDSEGMENTS     4
#define MAX_ENCODERS        15
#define MAX_STEPPERS        10
#define MAX_MFSERVOS        10
#define MAX_MFLCD_I2C       2
#define MAX_ANALOG_INPUTS   5           // check for warning!!
#define MAX_OUTPUT_SHIFTERS 4
#define MAX_INPUT_SHIFTERS  4
#define MAX_MCP_EXPANDER    4

#define STEPS               64
#define STEPPER_SPEED       400     // 300 already worked, 467, too?
#define STEPPER_ACCEL       800

#define NATIVE_MAX_PINS     41      // max Pin number! from module, w/o Port Expander
#define MCP_PIN_BASE        100     // first Pin number from Port Expander
#define MODULE_MAX_PINS     NATIVE_MAX_PINS

#define MOBIFLIGHT_TYPE         "MobiFlight Teensy41"
#define MOBIFLIGHT_SERIAL       "1234567890"
#define MOBIFLIGHT_NAME         "MobiFlight Teensy41"
#define EEPROM_SIZE             4248    // EEPROMSizeTeensy4 is not part of the EEPROMex lib!?
#define MEMLEN_CONFIG           1496    // MUST be less than EEPROM_SIZE!! MEM_OFFSET_CONFIG + MEM_LEN_CONFIG <= EEPROM_SIZE, see: eeprom_write_block (MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
#define MEMLEN_CONFIG_BUFFER    1000    // max. size for configBuffer, contains only names from inputs
#define MF_MAX_DEVICEMEM        1500    // max. memory size for devices

#define RANDOM_SEED_INPUT       A0

#endif