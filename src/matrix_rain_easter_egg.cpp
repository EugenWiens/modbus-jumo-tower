// SPDX-License-Identifier: GPL-3.0-or-later
#include "matrix_rain_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t RAIN_CYCLE_DURATION_MS = 5000;
constexpr uint32_t RAIN_STEP_DURATION_MS = 85;
constexpr uint8_t COLUMN_COUNT = 25;
constexpr uint8_t ROW_COUNT = 18;
constexpr uint8_t TAIL_LENGTH = 7;
constexpr uint16_t DARK_GREEN = 0x01E0;
constexpr uint16_t MID_GREEN = 0x03E0;
constexpr uint16_t JUMO_BLUE = 0x02D8;

char matrixCharacter(uint16_t value)
{
    constexpr char characters[] = "0123456789ABCDEFJUMO<>[]{}";
    return characters[value % (sizeof(characters) - 1U)];
}

void drawRain(Adafruit_ST7735& display, uint32_t elapsedMs, uint8_t displayIdx)
{
    const uint32_t step = elapsedMs / RAIN_STEP_DURATION_MS;
    display.setTextSize(1);

    for (uint8_t column = 0; column < COLUMN_COUNT; column++)
    {
        const uint8_t offset = static_cast<uint8_t>((column * 7U + displayIdx * 5U) % ROW_COUNT);
        const uint8_t headRow = static_cast<uint8_t>((step + offset) % (ROW_COUNT + TAIL_LENGTH));

        for (uint8_t tail = 0; tail < TAIL_LENGTH; tail++)
        {
            if (headRow < tail)
            {
                continue;
            }

            const uint8_t row = static_cast<uint8_t>(headRow - tail);
            if (row >= ROW_COUNT)
            {
                continue;
            }

            const uint16_t color = tail == 0U ? ST77XX_GREEN : tail < 3U ? MID_GREEN : DARK_GREEN;
            display.setTextColor(color);
            display.setCursor(static_cast<int16_t>(column * 6U + 4U), static_cast<int16_t>(row * 7U));
            display.print(matrixCharacter(static_cast<uint16_t>(step + column * 11U + row * 3U + tail)));
        }
    }
}

void drawJumo(Adafruit_ST7735& display, uint32_t cycleElapsedMs)
{
    const bool pulse = ((cycleElapsedMs - 1600U) / 180U) % 2U == 0U;
    display.fillRect(16, 39, 128, 49, ST77XX_BLACK);
    display.setTextColor(pulse ? JUMO_BLUE : ST77XX_CYAN);
    display.setTextSize(5);
    display.setCursor(20, 44);
    display.print("JUMO");
}
}  // namespace

void MatrixRainEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool MatrixRainEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void MatrixRainEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    const uint32_t cycleElapsedMs = elapsedMs % RAIN_CYCLE_DURATION_MS;
    display.fillScreen(ST77XX_BLACK);
    drawRain(display, elapsedMs, displayIdx);

    if (cycleElapsedMs >= 1600U && cycleElapsedMs < 3900U)
    {
        drawJumo(display, cycleElapsedMs);
    }
}