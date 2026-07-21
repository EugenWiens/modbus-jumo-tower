// SPDX-License-Identifier: GPL-3.0-or-later
#include "temperature_rocket_easter_egg.h"

#include <Adafruit_ST7735.h>
#include <stdio.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t HEAT_UP_DURATION_MS = 3000;
constexpr uint32_t COUNTDOWN_DURATION_MS = 1500;
constexpr uint32_t LAUNCH_DURATION_MS = 3500;
constexpr int16_t ROCKET_X = DISP_WIDTH / 2;
constexpr int16_t ROCKET_START_Y = DISP_HEIGHT - 36;

void drawFlame(Adafruit_ST7735& display, int16_t x, int16_t y, uint32_t elapsedMs)
{
    const bool brightFlame = (elapsedMs / 80U) % 2U == 0U;
    display.fillTriangle(x - 5, y + 10, x + 5, y + 10, x, y + 22, ST77XX_RED);
    display.fillTriangle(x - 3, y + 10, x + 3, y + 10, x, y + 17,
                         brightFlame ? ST77XX_YELLOW : ST77XX_ORANGE);
}

void drawRocket(Adafruit_ST7735& display, int16_t centerX, int16_t topY, uint32_t elapsedMs)
{
    display.fillTriangle(centerX - 11, topY + 17, centerX + 11, topY + 17, centerX, topY, ST77XX_WHITE);
    display.fillRect(centerX - 11, topY + 17, 23, 25, ST77XX_WHITE);
    display.fillTriangle(centerX - 11, topY + 31, centerX - 20, topY + 44, centerX - 11, topY + 42,
                         ST77XX_RED);
    display.fillTriangle(centerX + 11, topY + 31, centerX + 20, topY + 44, centerX + 11, topY + 42,
                         ST77XX_RED);
    display.fillCircle(centerX, topY + 22, 6, ST77XX_BLUE);
    display.drawCircle(centerX, topY + 22, 6, ST77XX_CYAN);
    drawFlame(display, centerX, static_cast<int16_t>(topY + 42), elapsedMs);
}

void drawTemperatureGauge(Adafruit_ST7735& display, uint32_t elapsedMs)
{
    const uint16_t temperature = static_cast<uint16_t>(20U + (elapsedMs * 79U) / HEAT_UP_DURATION_MS);
    char temperatureText[8];
    snprintf(temperatureText, sizeof(temperatureText), "%uC", temperature);

    display.setTextSize(3);
    display.setTextColor(ST77XX_WHITE);
    const int16_t textX = static_cast<int16_t>((DISP_WIDTH - strlen(temperatureText) * 18) / 2);
    display.setCursor(textX, 10);
    display.print(temperatureText);

    constexpr int16_t gaugeLeft = 25;
    constexpr int16_t gaugeTop = 45;
    constexpr int16_t gaugeWidth = 110;
    constexpr int16_t gaugeHeight = 12;
    const int16_t filledWidth = static_cast<int16_t>((gaugeWidth * elapsedMs) / HEAT_UP_DURATION_MS);
    display.drawRect(gaugeLeft, gaugeTop, gaugeWidth, gaugeHeight, ST77XX_WHITE);
    display.fillRect(gaugeLeft + 1, gaugeTop + 1, filledWidth, gaugeHeight - 2,
                     elapsedMs < HEAT_UP_DURATION_MS / 2U ? ST77XX_GREEN : ST77XX_RED);
}
}  // namespace

void TemperatureRocketEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                       uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool TemperatureRocketEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void TemperatureRocketEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display,
                                              uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    display.fillScreen(ST77XX_BLACK);

    if (elapsedMs < HEAT_UP_DURATION_MS)
    {
        drawTemperatureGauge(display, elapsedMs);
        drawRocket(display, ROCKET_X, ROCKET_START_Y, elapsedMs);
        return;
    }

    if (elapsedMs < HEAT_UP_DURATION_MS + COUNTDOWN_DURATION_MS)
    {
        const uint32_t remainingMs = HEAT_UP_DURATION_MS + COUNTDOWN_DURATION_MS - elapsedMs;
        const uint8_t countdown = static_cast<uint8_t>((remainingMs + 499U) / 500U);
        display.setTextColor(ST77XX_RED);
        display.setTextSize(7);
        display.setCursor(60, 35);
        display.print(countdown);
        drawRocket(display, ROCKET_X, ROCKET_START_Y, elapsedMs);
        return;
    }

    const uint32_t launchElapsedMs = elapsedMs - HEAT_UP_DURATION_MS - COUNTDOWN_DURATION_MS;
    if (launchElapsedMs < LAUNCH_DURATION_MS)
    {
        const int16_t rocketY = static_cast<int16_t>(ROCKET_START_Y -
                                                     ((ROCKET_START_Y + 50) * launchElapsedMs) /
                                                         LAUNCH_DURATION_MS);
        drawRocket(display, ROCKET_X, rocketY, elapsedMs);
    }
}