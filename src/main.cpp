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
static uint16_t s_appliedTempHigh = TEMP_REG_DISABLED;
static uint16_t s_appliedTempLow = TEMP_REG_DISABLED;
static uint16_t s_appliedHumidityHigh = TEMP_REG_DISABLED;
static uint16_t s_appliedHumidityLow = TEMP_REG_DISABLED;
static uint16_t s_pendingTempHigh = TEMP_REG_DISABLED;
static uint16_t s_pendingTempLow = TEMP_REG_DISABLED;
static uint16_t s_pendingHumidityHigh = TEMP_REG_DISABLED;
static uint16_t s_pendingHumidityLow = TEMP_REG_DISABLED;
static bool s_climatePending = false;
static uint32_t s_climatePendingSinceMs = 0;
static bool s_climateModeActive = false;
static uint16_t s_ledToggleLoopCount = 0;
static bool s_prevLedState = false;

constexpr uint32_t CLIMATE_STABILIZATION_MS = 60U;

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
    g_display.showBootLogo(DISPLAY_LEFT_INDEX);
    g_display.showLargeText(DISPLAY_FRONT_INDEX, 4, "Hallo", "Harald");
    char firmwareVersion[16];
    snprintf(firmwareVersion, sizeof(firmwareVersion), "%d.%d.%d", FIRMWARE_VERSION_MAJOR,
             FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
    g_display.showLargeText(DISPLAY_RIGHT_INDEX, 2, "SW Version", firmwareVersion);
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
    const uint32_t nowMs = millis();

    // ── Easter Egg command register ──────────────────────────────────────────
    const uint16_t requestedEasterEgg = g_modbus.holdingRegs[REG_EASTER_EGG];
    if (requestedEasterEgg != EASTER_EGG_NONE)
    {
        // Commands are self-resetting so writing the same Egg ID can trigger it again.
        g_modbus.holdingRegs[REG_EASTER_EGG] = EASTER_EGG_NONE;
        if (g_display.startEasterEgg(requestedEasterEgg, nowMs))
        {
            DBG_SERIAL.printf("Easter Egg %u: started\r\n", requestedEasterEgg);
        }
        else
        {
            DBG_SERIAL.printf("Easter Egg %u: unknown\r\n", requestedEasterEgg);
        }
    }
    g_display.updateEasterEgg(nowMs);

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
    if (curTempHigh != s_pendingTempHigh || curTempLow != s_pendingTempLow ||
        curHumidityHigh != s_pendingHumidityHigh || curHumidityLow != s_pendingHumidityLow)
    {
        s_pendingTempHigh = curTempHigh;
        s_pendingTempLow = curTempLow;
        s_pendingHumidityHigh = curHumidityHigh;
        s_pendingHumidityLow = curHumidityLow;
        s_climatePending = true;
        s_climatePendingSinceMs = nowMs;
    }

    if (s_climatePending && nowMs - s_climatePendingSinceMs >= CLIMATE_STABILIZATION_MS)
    {
        s_climatePending = false;
        const bool stableClimateChanged =
            s_pendingTempHigh != s_appliedTempHigh || s_pendingTempLow != s_appliedTempLow ||
            s_pendingHumidityHigh != s_appliedHumidityHigh ||
            s_pendingHumidityLow != s_appliedHumidityLow;
        if (stableClimateChanged)
        {
            s_appliedTempHigh = s_pendingTempHigh;
            s_appliedTempLow = s_pendingTempLow;
            s_appliedHumidityHigh = s_pendingHumidityHigh;
            s_appliedHumidityLow = s_pendingHumidityLow;

            const bool hasTemperature = g_modbus.hasTemperature();
            s_climateModeActive = hasTemperature;
            if (hasTemperature)
            {
                const float temperature = g_modbus.getTemperature();
                const bool hasHumidity = g_modbus.hasHumidity();
                const float humidity = g_modbus.getHumidity();
                DBG_SERIAL.printf("Temperature mode: %.1f C%s\r\n",
                                  static_cast<double>(temperature),
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
    }

    bool anyDisplayChanged = false;
    for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
    {
        anyDisplayChanged = anyDisplayChanged || dispChanged[dispIdx];
    }

    if (anyDisplayChanged)
    {
        for (uint8_t dispIdx = 0; dispIdx < DISPLAY_COUNT; dispIdx++)
        {
            if (dispChanged[dispIdx] && !s_climateModeActive)
            {
                refreshDisplay(dispIdx);
                DBG_SERIAL.printf("Display %u: text updated\r\n", dispIdx + 1U);
            }
        }
    }

    memcpy(s_prevRegs, g_modbus.holdingRegs, sizeof(s_prevRegs));
}
