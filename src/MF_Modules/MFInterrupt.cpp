#include <Arduino.h>

#ifndef USE_INTERRUPT

static long TimePrev_10ms = 0;

bool get_10ms_flag(void) {
    if (millis() - TimePrev_10ms > 10) {
        TimePrev_10ms = millis();
        return true;
    }
    return false;
}

void setup_interrupt(void) {
    return;
}

#else

#include "MFBoards.h"
#include "MFInterrupt.h"
#include "mobiflight.h"
#if MF_JOYSTICK_SUPPORT == 1
    #include "MFJoystick.h"
#endif

#if defined(ARDUINO_ARCH_AVR)
#include <TimerOne.h>
void timerIsr(void);
void setup_interrupt() {
    Timer1.initialize(10000);
    Timer1.attachInterrupt(timerIsr);
    return;
}
#elif defined(ARDUINO_ARCH_RP2040)
#include <pico/time.h>
bool timerIsr(struct repeating_timer *t);
struct repeating_timer timer;
void setup_interrupt() {
    add_repeating_timer_ms(-1, timerIsr, NULL, &timer);
    return;
}
#elif defined(ARDUINO_ARCH_STM32)
void timerIsr(void);
void setup_interrupt(void) {
#if defined(TIM1)
  TIM_TypeDef *Instance = TIM1;
#else
  TIM_TypeDef *Instance = TIM2;
#endif
  // Instantiate HardwareTimer object. Thanks to 'new' instanciation, HardwareTimer is not destructed when setup() function is finished.
  HardwareTimer *MyTim = new HardwareTimer(Instance);
  MyTim->setOverflow(1000, MICROSEC_FORMAT);
  MyTim->attachInterrupt(timerIsr);
  MyTim->resume();
}
#elif defined(CORE_TEENSY)
IntervalTimer myTimer;

void timerIsr(void);
void setup_interrupt(void) {
    myTimer.begin(timerIsr, 1000);
}
#elif defined(ARDUINO_ARCH_ESP8266)
//TBD
    #error interrupt not implemented yet
#endif

bool seconds_flag = false;
bool flag_1ms = false;
bool flag_10ms = false;
bool flag_100ms = false;

#if defined(ARDUINO_ARCH_RP2040)
bool timerIsr(struct repeating_timer *t) {
#else
void timerIsr(void) {
#endif
    static uint8_t Timer_10ms=0;
    static uint8_t Timer_100ms=0;
    static uint32_t	seconds = 0;
#if !defined(ARDUINO_ARCH_AVR)
    static uint8_t Timer_1ms=0;
    for (int i=0; i!=encodersRegistered; i++) {
        encoders[i].readInput();
    }
#if MF_JOYSTICK_SUPPORT == 1
    sample_adc();
#endif
    flag_1ms = true;
    Timer_1ms++;
    if (Timer_1ms==10) {
        Timer_1ms=0;
        Timer_10ms++;
        flag_10ms = true;
    }
    if (Timer_10ms==10) {
        Timer_10ms=0;
        Timer_100ms++;
        flag_100ms = true;
    }
#else
    flag_10ms = true;
    Timer_10ms++;
#endif
    if (Timer_100ms==10) {
        Timer_100ms=0;
        seconds++;
        seconds_flag = true;
    }
#if defined(ARDUINO_ARCH_RP2040)
    return true;
#endif
}

bool get_100ms_flag() {
  if (flag_100ms) {
      flag_100ms = false;
      return true;
  }
  return false;        
}

bool get_10ms_flag() {
  if (flag_10ms) {
      flag_10ms = false;
      return true;
  }
  return false;        
}

#endif
