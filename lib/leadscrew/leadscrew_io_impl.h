
#include <Wire.h>

#include "leadscrew_io.h"
#pragma once

class LeadscrewIOImpl : public LeadscrewIO {
  inline void writeStepPin(uint8_t val) {
#ifdef ESP32    
    digitalWrite(ELS_LEADSCREW_STEP, val);
#else
    digitalWriteFast(ELS_LEADSCREW_STEP, val);
#endif
  }
#ifdef ESP32    
  inline uint8_t readStepPin() { return digitalRead(ELS_LEADSCREW_STEP); }
#else
  inline uint8_t readStepPin() { return digitalReadFast(ELS_LEADSCREW_STEP); }
#endif

  inline void writeDirPin(uint8_t val) {
#ifdef ESP32
    if(val == 1)
      REG_SET_BIT(GPIO_OUT_REG, ELS_LEADSCREW_DIR_BIT);
    else
      REG_CLR_BIT(GPIO_OUT_REG, ELS_LEADSCREW_DIR_BIT);
#else
    digitalWriteFast(ELS_LEADSCREW_DIR, val);
#endif
  }
#ifdef ESP32    
  inline u_int8_t readDirPin() { return digitalRead(ELS_LEADSCREW_DIR); }
#else
  inline u_int8_t readDirPin() { return digitalReadFast(ELS_LEADSCREW_DIR); }
#endif
};                                                                                          