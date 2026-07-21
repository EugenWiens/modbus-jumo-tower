// SPDX-License-Identifier: GPL-3.0-or-later
#include "debug_console_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t LOG_INTERVAL_MS = 700;
constexpr uint8_t MAX_VISIBLE_LOGS = 6;
constexpr uint16_t JUMO_BLUE = 0x02D8;

constexpr const char* ROLES[] = {"DEV", "TEST", "SHIP"};
constexpr const char* LOG_MESSAGES[] = {
    "boot JUMO Tower",
    "init Modbus... OK",
    "scan GPIO... OK",
    "coffee check... OK",
    "build firmware... OK",
    "tests: 42/42",
    "deploying... OK",
    "no bugs found",
    "STATUS: LEGENDARY",
};
constexpr uint8_t LOG_MESSAGE_COUNT = sizeof(LOG_MESSAGES) / sizeof(LOG_MESSAGES[0]);

void drawHeader(Adafruit_ST7735& display, uint8_t displayIdx)
{
    display.setTextSize(1);
    display.setTextColor(JUMO_BLUE);
    display.setCursor(4, 7);
    display.print("JUMO");
    display.setTextColor(ST77XX_WHITE);
    display.print(" CONSOLE ");
    display.setTextColor(ST77XX_CYAN);
    display.print(ROLES[displayIdx % (sizeof(ROLES) / sizeof(ROLES[0]))]);
    display.drawFastHLine(4, 19, DISP_WIDTH - 8, JUMO_BLUE);
}
}  // namespace

void DebugConsoleEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                  uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool DebugConsoleEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void DebugConsoleEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    uint8_t visibleLogCount = static_cast<uint8_t>(elapsedMs / LOG_INTERVAL_MS + 1U);
    if (visibleLogCount > LOG_MESSAGE_COUNT)
    {
        visibleLogCount = LOG_MESSAGE_COUNT;
    }
    const uint8_t firstLog = visibleLogCount > MAX_VISIBLE_LOGS ? visibleLogCount - MAX_VISIBLE_LOGS : 0;

    display.fillScreen(ST77XX_BLACK);
    drawHeader(display, displayIdx);
    display.setTextSize(1);

    for (uint8_t logIdx = firstLog; logIdx < visibleLogCount; logIdx++)
    {
        const uint8_t visibleRow = logIdx - firstLog;
        const bool isFinalStatus = logIdx == LOG_MESSAGE_COUNT - 1U;
        display.setTextColor(isFinalStatus ? JUMO_BLUE : ST77XX_GREEN);
        display.setCursor(4, static_cast<int16_t>(28U + visibleRow * 13U));
        display.print("> ");
        display.print(LOG_MESSAGES[logIdx]);
    }

    if ((elapsedMs / 250U) % 2U == 0U)
    {
        display.setTextColor(ST77XX_WHITE);
        display.setCursor(4, 112);
        display.print("_");
    }
}