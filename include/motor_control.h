#pragma once

#include <Arduino.h>

class MotorControl {
  public:
    void init(uint8_t pin);
    void set(bool on);

  private:
    uint8_t _pin = 0;
};
