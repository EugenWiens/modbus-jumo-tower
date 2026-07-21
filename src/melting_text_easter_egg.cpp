// SPDX-License-Identifier: GPL-3.0-or-later
#include "melting_text_easter_egg.h"

#include <Adafruit_ST7735.h>

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t CHARACTER_FALL_DURATION_MS = 1800;
constexpr uint32_t LINE_DELAY_MS = 350;
constexpr uint32_t CHARACTER_DELAY_MS = 90;
constexpr uint16_t CHARACTER_MELT_PROGRESS = 700;
}

void MeltingTextEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                 uint8_t displayCount)
{
    _startMs = startMs;
    _displayCount = displayCount;
    for (uint8_t displayIdx = 0; displayIdx < _displayCount; displayIdx++)
    {
        _snapshots[displayIdx] = snapshots[displayIdx];
    }
}

bool MeltingTextEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void MeltingTextEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    display.fillScreen(ST77XX_BLACK);
    display.setTextColor(ST77XX_WHITE);

    const EasterEggDisplaySnapshot& snapshot = _snapshots[displayIdx];
    for (uint8_t lineIdx = 0; lineIdx < EASTER_EGG_MAX_LINES; lineIdx++)
    {
        const EasterEggTextLine& line = snapshot.lines[lineIdx];
        display.setTextSize(line.textSizeX, line.textSizeY);
        const int16_t characterWidth = static_cast<int16_t>(6U * line.textSizeX);
        const int16_t characterHeight = static_cast<int16_t>(8U * line.textSizeY);

        for (uint8_t characterIdx = 0; line.text[characterIdx] != '\0'; characterIdx++)
        {
            const char character = line.text[characterIdx];
            if (character == ' ')
            {
                continue;
            }

            const uint32_t startDelay = lineIdx * LINE_DELAY_MS + characterIdx * CHARACTER_DELAY_MS;
            const uint32_t fallElapsed = elapsedMs > startDelay ? elapsedMs - startDelay : 0;
            const uint16_t progress = fallElapsed >= CHARACTER_FALL_DURATION_MS
                                          ? 1000U
                                          : static_cast<uint16_t>((fallElapsed * 1000U) /
                                                                  CHARACTER_FALL_DURATION_MS);
            const int16_t targetY = static_cast<int16_t>(display.height() - characterHeight);
            const int16_t verticalDistance = static_cast<int16_t>(targetY - line.y);
            const int16_t y = static_cast<int16_t>(line.y + (verticalDistance * progress) / 1000);
            const int16_t sway = progress == 0U
                                    ? 0
                                    : static_cast<int16_t>((elapsedMs / 140U + characterIdx * 3U + lineIdx) %
                                                           5U) -
                                          2;
            const int16_t x = static_cast<int16_t>(line.x + characterIdx * characterWidth + sway);

            if (progress < CHARACTER_MELT_PROGRESS)
            {
                display.setCursor(x, y);
                display.print(character);
            }

            if (progress >= 250U)
            {
                const int16_t dropletLength = static_cast<int16_t>(progress / 100U);
                display.drawFastVLine(static_cast<int16_t>(x + characterWidth / 2),
                                      static_cast<int16_t>(y + characterHeight - 1), dropletLength,
                                      ST77XX_WHITE);
            }
        }
    }
}