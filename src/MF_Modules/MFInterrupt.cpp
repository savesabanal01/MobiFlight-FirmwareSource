#include <Arduino.h>

//#ifdef USE_INTERRUPT

#include "MFBoards.h"
#include "Button.h"
#include "./MF_Encoder/Encoder.h"     // otherwise Teensy specific Encoder lib is used
#if MF_ANALOG_SUPPORT == 1
#include "Analog.h"
#endif
#if MF_SERVO_SUPPORT == 1
#include "Servos.h"
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
#include "InputShifter.h"
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
#include "DigInMux.h"
#endif

#if defined(ARDUINO_ARCH_AVR)
#include <TimerOne.h>
void timerIsr(void);
void setup_interrupt()
{
    Timer1.initialize(10000);
    Timer1.attachInterrupt(timerIsr);
    return;
}
#elif defined(ARDUINO_ARCH_RP2040)
#include <pico/time.h>
bool                   timerIsr(struct repeating_timer *t);
struct repeating_timer timer;
void                   setup_interrupt()
{
    add_repeating_timer_ms(-1, timerIsr, NULL, &timer);
    return;
}
#elif defined(CORE_TEENSY)
IntervalTimer myTimer;

void timerIsr(void);
void setup_interrupt(void)
{
    myTimer.begin(timerIsr, 1000);
}
#endif

#if defined(ARDUINO_ARCH_RP2040)
bool timerIsr(struct repeating_timer *t)
{
#else
void timerIsr(void)
{
#endif
    static uint8_t  Timer_1ms   = 0;

    Encoder::tick();
    Timer_1ms++;
    if (!(Timer_1ms % 10)) {
        Button::read();
    }
#if MF_SERVO_SUPPORT == 1
    if ((Timer_1ms - 2) % 5 == 0) {
        Servos::update();
    }
#endif
#if MF_ANALOG_SUPPORT == 1
    if ((Timer_1ms - 4) % 10 == 0) {
        Analog::readAverage();
    }
#endif
#if MF_INPUT_SHIFTER_SUPPORT == 1
    if ((Timer_1ms - 6) % 10 == 0) {
        // InputShifter read in, tbd.
    }
#endif
#if MF_DIGIN_MUX_SUPPORT == 1
    if ((Timer_1ms - 6) % 10 == 0) {
        // Mux read in, tbd.
    }
#endif
    if (Timer_1ms == 10) {
        Timer_1ms = 0;
    }
#if defined(ARDUINO_ARCH_RP2040)
    return true;
#endif
}

//#endif