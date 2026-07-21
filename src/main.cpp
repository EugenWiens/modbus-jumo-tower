// SPDX-License-Identifier: GPL-3.0-or-later
#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
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
static uint16_t s_prevTempHigh = TEMP_REG_DISABLED;
static uint16_t s_prevTempLow = TEMP_REG_DISABLED;
static uint16_t s_prevHumidityHigh = TEMP_REG_DISABLED;
static uint16_t s_prevHumidityLow = TEMP_REG_DISABLED;
static uint16_t s_ledToggleLoopCount = 0;
static bool s_prevLedState = false;

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
#ifdef DEBUG
    delay(2000);
#endif
    DBG_SERIAL.println("Starting JUMO Tower");

    g_display.init();
    g_display.showLargeText(0, 4, "Hallo", "Harald");
    char firmwareVersion[16];
    snprintf(firmwareVersion, sizeof(firmwareVersion), "%d.%d.%d", FIRMWARE_VERSION_MAJOR,
             FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
    g_display.showLargeText(1, 2, "SW Version", firmwareVersion);
    DBG_SERIAL.printf("Displays initialized: %u ST7735 TFTs\r\n", DISPLAY_COUNT);
    g_motor.init(MOTOR_PIN);
    DBG_SERIAL.printf("Motor initialized: GPIO %u\r\n", MOTOR_PIN);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    g_modbus.begin();
    DBG_SERIAL.printf("Modbus initialized: unit=%u, baud=%lu\r\n", MODBUS_UNIT_ID,
                      static_cast<unsigned long>(MODBUS_BAUD));

    DBG_SERIAL.printf("FW %d.%d.%d\r\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR,
                      FIRMWARE_VERSION_PATCH);
}

void loop()
{
    g_modbus.poll();

    // ── Easter Egg command register ──────────────────────────────────────────
    const uint16_t requestedEasterEgg = g_modbus.holdingRegs[REG_EASTER_EGG];
    if (requestedEasterEgg != EASTER_EGG_NONE)
    {
        // Commands are self-resetting so writing the same Egg ID can trigger it again.
        g_modbus.holdingRegs[REG_EASTER_EGG] = EASTER_EGG_NONE;
        if (g_display.startEasterEgg(requestedEasterEgg, millis()))
        {
            DBG_SERIAL.printf("Easter Egg %u: started\r\n", requestedEasterEgg);
        }
        else
        {
            DBG_SERIAL.printf("Easter Egg %u: unknown\r\n", requestedEasterEgg);
        }
    }
    g_display.updateEasterEgg(millis());

    if (++s_ledToggleLoopCount == 10000U)
    {
        s_ledToggleLoopCount = 0;
        s_prevLedState = !s_prevLedState;
        digitalWrite(LED_BUILTIN, s_prevLedState ? HIGH : LOW);
    }

    // ── Motor coil ────────────────────────────────────────────────────────────
    const bool curMotor = g_modbus.getMotorState();
    if (curMotor != s_prevMotor)
    {
        s_prevMotor = curMotor;
        g_motor.set(curMotor);
        DBG_SERIAL.printf("Motor: %s\r\n", curMotor ? "ON" : "OFF");
    }

    // ── Display holding registers ─────────────────────────────────────────────
    bool dispChanged[DISPLAY_COUNT] = {false};
    for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
    {
        const uint8_t firstReg = DISPLAY_REG_BASES[dispIdx];
        for (uint8_t regOffset = 0; regOffset < 8; regOffset++)
        {
            const uint8_t regIdx = static_cast<uint8_t>(firstReg + regOffset);
            if (g_modbus.holdingRegs[regIdx] != s_prevRegs[regIdx])
            {
                dispChanged[dispIdx] = true;
            }
        }
    }

    // ── Climate registers (overrides all displays when temperature is set) ───
    const uint16_t curTempHigh = g_modbus.holdingRegs[REG_TEMPERATURE_HIGH];
    const uint16_t curTempLow = g_modbus.holdingRegs[REG_TEMPERATURE_LOW];
    const uint16_t curHumidityHigh = g_modbus.holdingRegs[REG_HUMIDITY_HIGH];
    const uint16_t curHumidityLow = g_modbus.holdingRegs[REG_HUMIDITY_LOW];
    if (curTempHigh != s_prevTempHigh || curTempLow != s_prevTempLow ||
        curHumidityHigh != s_prevHumidityHigh || curHumidityLow != s_prevHumidityLow)
    {
        s_prevTempHigh = curTempHigh;
        s_prevTempLow = curTempLow;
        s_prevHumidityHigh = curHumidityHigh;
        s_prevHumidityLow = curHumidityLow;
        if (g_modbus.hasTemperature())
        {
            const float temperature = g_modbus.getTemperature();
            const bool hasHumidity = g_modbus.hasHumidity();
            const float humidity = g_modbus.getHumidity();
            DBG_SERIAL.printf("Temperature mode: %.1f C%s\r\n", static_cast<double>(temperature),
                              hasHumidity ? " with humidity" : "");
            for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
            {
                if (hasHumidity)
                {
                    g_display.showClimate(dispIdx, temperature, humidity);
                }
                else
                {
                    g_display.showTemperature(dispIdx, temperature);
                }
            }
        }
        else
        {
            DBG_SERIAL.println("Temperature mode: disabled");
            for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
            {
                refreshDisplay(dispIdx);
            }
        }
        for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
        {
            dispChanged[dispIdx] = false;
        }
    }

    bool anyDisplayChanged = false;
    for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
    {
        anyDisplayChanged = anyDisplayChanged || dispChanged[dispIdx];
    }

    if (anyDisplayChanged)
    {
        memcpy(s_prevRegs, g_modbus.holdingRegs, sizeof(s_prevRegs));
        for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
        {
            if (dispChanged[dispIdx] && !g_modbus.hasTemperature())
            {
                refreshDisplay(dispIdx);
                DBG_SERIAL.printf("Display %u: text updated\r\n", dispIdx + 1U);
            }
        }
    }
}
