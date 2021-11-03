#ifndef MFBoardMicro_h
#define MFBoardMicro_h

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
#ifndef MF_SHIFTER_SUPPORT
#define MF_SHIFTER_SUPPORT 1
#endif

#define MF_MCP_SUPPORT      0

#define MAX_OUTPUTS         15
#define MAX_BUTTONS         26
#define MAX_LEDSEGMENTS     2
#define MAX_ENCODERS        12
#define MAX_STEPPERS        4
#define MAX_MFSERVOS        4
#define MAX_MFLCD_I2C       2
#define MAX_ANALOG_INPUTS   3
#define MAX_SHIFTERS        4
#define MAX_MCP_EXPANDER    4

#define STEPS               64
#define STEPPER_SPEED       400     // 300 already worked, 467, too?
#define STEPPER_ACCEL       800

#define NATIVE_MAX_PINS     28      // max Pin number! from module, w/o Port Expander
#define MCP_PIN_BASE        100     // first Pin number from Port Expander
#define MODULE_MAX_PINS     NATIVE_MAX_PINS + ((MAX_MCP_EXPANDER * 16) * MF_MCP_SUPPORT)

#if MF_MCP_SUPPORT == 1
#include <MFMCP23017.h>
extern MFMCP23017 mcp_expander[];
#endif

#define MOBIFLIGHT_TYPE         "MobiFlight RaspiPico"
#define MOBIFLIGHT_SERIAL       "0987654321"
#define MOBIFLIGHT_NAME         "MobiFlight RaspiPico"
#define EEPROM_SIZE             2048    // EEPROMSizeRaspberryPico
#define MEMLEN_CONFIG           1024    // MUST be less than EEPROM_SIZE!! MEM_OFFSET_CONFIG + MEM_LEN_CONFIG <= EEPROM_SIZE, see: eeprom_write_block (MEM_OFFSET_CONFIG, configBuffer, MEM_LEN_CONFIG);
#define RANDOM_SEED_INPUT       A0

#define SDA                     PIN_WIRE0_SDA
#define SCL                     PIN_WIRE0_SCL

#endif
