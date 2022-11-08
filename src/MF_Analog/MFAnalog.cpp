//
// MFAnalog.cpp
//
// (C) MobiFlight Project 2022
//

#include "MFAnalog.h"
#include "MFEEPROM.h"
#include "commandmessenger.h"

extern MFEEPROM MFeeprom;

analogEvent MFAnalog::_handler = NULL;

struct {
    uint16_t minValue = 512;
    uint16_t maxValue = 512;
} CalibrationData;

MFAnalog::MFAnalog(uint8_t pin, const char *name, uint8_t sensitivity)
{
    _sensitivity = sensitivity;
    _pin         = pin;
    _name        = name;
    pinMode(_pin, INPUT_PULLUP); // set pin to input. Could use OUTPUT for analog, but shows the intention :-)
    // Fill averaging buffers with initial reading
    for (uint8_t i = 0; i < ADC_MAX_AVERAGE; i++) {
        readBuffer();
    }
    // and set initial value from buffers
    _lastValue = ADC_Average_Total >> ADC_MAX_AVERAGE_LOG2;
}

bool MFAnalog::valueHasChanged(int16_t newValue)
{
    return (abs(newValue - _lastValue) >= _sensitivity);
}

void MFAnalog::readChannel(uint8_t alwaysTrigger)
{
    int16_t newValue = ADC_Average_Total >> ADC_MAX_AVERAGE_LOG2;
    if (alwaysTrigger || valueHasChanged(newValue)) {
        _lastValue = newValue;
        if (_handler != NULL) {
            (*_handler)(_lastValue, _pin, _name);
        }
    }
}

void MFAnalog::update()
{
    readChannel(false);
}

void MFAnalog::retrigger()
{
    readChannel(true);
}

void MFAnalog::doCalibration()
{
    uint32_t startMillis = millis();
    uint16_t actualValue = 0;

    // read in the min and max value, poti must be moved
    while (startMillis < millis() + CALIBRATION_TIME) {
        // fill the average buffer every 10ms as we are in a while loop and it is not done in the main loop()
        if (millis() % 10)
            readBuffer();
        actualValue = ADC_Average_Total >> ADC_MAX_AVERAGE_LOG2;
        if (actualValue < CalibrationData.minValue)
            CalibrationData.minValue = actualValue;
        if (actualValue > CalibrationData.maxValue)
            CalibrationData.maxValue = actualValue;
    }
    // store min and max value to EEPROM, consider pin number for EEPROM adress
    if (!MFeeprom.write_block(CALIBRATION_START_ADRESS + (_pin - FIRST_ANALOG_PIN) * sizeof(CalibrationData), CalibrationData)) {
        cmdMessenger.sendCmd(kStatus, F("Failure on writing analog calibration data"));
    }
}

void MFAnalog::sendCalibration()
{
    if (!MFeeprom.read_block(CALIBRATION_START_ADRESS + (_pin - FIRST_ANALOG_PIN) * sizeof(CalibrationData), CalibrationData))
        cmdMessenger.sendCmd(kStatus, F("Failure on reading analog calibration data"));
    // check if calibration has been done
    if (CalibrationData.minValue > 1023 || CalibrationData.maxValue > 1023 || CalibrationData.maxValue <= CalibrationData.minValue) {
        CalibrationData.minValue = 0;
        CalibrationData.maxValue = 1023;
    }
    cmdMessenger.sendCmdStart(kReadAnalogCalibration);
    cmdMessenger.sendCmdArg(_name);
    cmdMessenger.sendCmdArg(CalibrationData.minValue);
    cmdMessenger.sendCmdArg(CalibrationData.maxValue);
    cmdMessenger.sendCmdEnd();
}

void MFAnalog::readBuffer()
{                                                           // read ADC and calculate floating average, call it every ~10ms
    ADC_Average_Total -= ADC_Buffer[(ADC_Average_Pointer)]; // subtract oldest value to save the newest value
    ADC_Buffer[ADC_Average_Pointer] = analogRead(_pin);     // store read in, must be subtracted in next loop
    ADC_Average_Total += ADC_Buffer[ADC_Average_Pointer];   // add read in for floating average
    ADC_Average_Pointer++;                                  // prepare for next loop
    ADC_Average_Pointer &= (ADC_MAX_AVERAGE - 1);           // limit max. values for floating average
}

void MFAnalog::attachHandler(analogEvent newHandler)
{
    _handler = newHandler;
}

// MFAnalog.cpp