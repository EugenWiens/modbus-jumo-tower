// SPDX-License-Identifier: GPL-3.0-or-later
#include "motor_control.h"

void MotorControl::init(uint8_t pin)
{
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
}

void MotorControl::set(bool on)
{
  digitalWrite(_pin, on ? HIGH : LOW);
}
