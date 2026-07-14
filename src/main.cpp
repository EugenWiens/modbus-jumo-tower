// SPDX-License-Identifier: GPL-3.0-or-later
#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "config.h"
#include "display_manager.h"
#include "modbus_handler.h"
#include "motor_control.h"

static ModbusHandler g_modbus;
static DisplayManager g_display;
static MotorControl g_motor;

static bool s_prevMotor = false;
static uint16_t s_prevRegs[MODBUS_NUM_REGS] = {0};
static uint16_t s_prevTemp = TEMP_REG_DISABLED;

static void refreshDisplay(uint8_t dispIdx)
{
  char line1[9];
  char line2[9];
  g_modbus.getDisplayText(dispIdx, 0, line1);
  g_modbus.getDisplayText(dispIdx, 1, line2);
  g_display.update(dispIdx, line1, line2);
}

void setup()
{
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();

  g_display.init(DISP1_I2C_ADDR, DISP2_I2C_ADDR);
  g_motor.init(MOTOR_PIN);
  g_modbus.begin();
}

void loop()
{
  g_modbus.poll();

  // ── Motor coil ────────────────────────────────────────────────────────────
  const bool curMotor = g_modbus.getMotorState();
  if (curMotor != s_prevMotor)
  {
    s_prevMotor = curMotor;
    g_motor.set(curMotor);
  }

  // ── Display holding registers ─────────────────────────────────────────────
  bool disp1Changed = false;
  bool disp2Changed = false;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (g_modbus.holdingRegs[i] != s_prevRegs[i])
    {
      disp1Changed = true;
    }
    if (g_modbus.holdingRegs[i + 8] != s_prevRegs[i + 8])
    {
      disp2Changed = true;
    }
  }

  // ── Temperature register (overrides display 1 text when set) ─────────────
  const uint16_t curTemp = g_modbus.holdingRegs[REG_TEMPERATURE];
  if (curTemp != s_prevTemp)
  {
    s_prevTemp = curTemp;
    if (g_modbus.hasTemperature())
    {
      g_display.showTemperature(0, g_modbus.getTemperature());
    }
    else
    {
      refreshDisplay(0);  // temperature disabled: restore text
    }
    disp1Changed = false;  // already handled
  }

  if (disp1Changed || disp2Changed)
  {
    memcpy(s_prevRegs, g_modbus.holdingRegs, sizeof(s_prevRegs));
    if (disp1Changed && !g_modbus.hasTemperature())
    {
      refreshDisplay(0);
    }
    if (disp2Changed)
    {
      refreshDisplay(1);
    }
  }
}
