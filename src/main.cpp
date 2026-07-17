// SPDX-License-Identifier: GPL-3.0-or-later
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "config.h"
#include "display_manager.h"
#include "modbus_handler.h"
#include "motor_control.h"

Adafruit_USBD_CDC DBG_SERIAL;

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
    DBG_SERIAL.begin(115200);  // register 2nd CDC interface before stack starts
    DBG_SERIAL.println("Starting JUMO Tower");

    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
    DBG_SERIAL.printf("I2C: SDA=GPIO %u, SCL=GPIO %u\r\n", I2C_SDA_PIN, I2C_SCL_PIN);

    g_display.init(DISP1_I2C_ADDR, DISP2_I2C_ADDR);
    DBG_SERIAL.printf("Displays initialized: 0x%02X, 0x%02X\r\n", DISP1_I2C_ADDR, DISP2_I2C_ADDR);
    g_motor.init(MOTOR_PIN);
    DBG_SERIAL.printf("Motor initialized: GPIO %u\r\n", MOTOR_PIN);
    g_modbus.begin();
    DBG_SERIAL.printf("Modbus initialized: unit=%u, baud=%lu\r\n", MODBUS_UNIT_ID,
                      static_cast<unsigned long>(MODBUS_BAUD));

    DBG_SERIAL.printf("FW %d.%d.%d\r\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR,
                      FIRMWARE_VERSION_PATCH);
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
        DBG_SERIAL.printf("Motor: %s\r\n", curMotor ? "ON" : "OFF");
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

    // ── Temperature register (overrides both displays when set) ───────────────
    const uint16_t curTemp = g_modbus.holdingRegs[REG_TEMPERATURE];
    if (curTemp != s_prevTemp)
    {
        s_prevTemp = curTemp;
        if (g_modbus.hasTemperature())
        {
            DBG_SERIAL.printf("Temperature mode: %.1f C\r\n",
                              static_cast<double>(g_modbus.getTemperature()));
            g_display.showTemperature(0, g_modbus.getTemperature());
            g_display.showTemperature(1, g_modbus.getTemperature());
        }
        else
        {
            DBG_SERIAL.println("Temperature mode: disabled");
            refreshDisplay(0);  // temperature disabled: restore text
            refreshDisplay(1);
        }
        disp1Changed = false;  // already handled
        disp2Changed = false;
    }

    if (disp1Changed || disp2Changed)
    {
        memcpy(s_prevRegs, g_modbus.holdingRegs, sizeof(s_prevRegs));
        if (disp1Changed && !g_modbus.hasTemperature())
        {
            refreshDisplay(0);
            DBG_SERIAL.println("Display 1: text updated");
        }
        if (disp2Changed && !g_modbus.hasTemperature())
        {
            refreshDisplay(1);
            DBG_SERIAL.println("Display 2: text updated");
        }
    }
}
