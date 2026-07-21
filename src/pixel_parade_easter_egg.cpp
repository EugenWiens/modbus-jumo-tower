// SPDX-License-Identifier: GPL-3.0-or-later
#include "pixel_parade_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t MARCH_CYCLE_DURATION_MS = 2600;
constexpr uint32_t SHOT_CYCLE_DURATION_MS = 900;
constexpr uint8_t FORMATION_COLUMNS = 5;
constexpr uint8_t FORMATION_ROWS = 3;
constexpr int16_t FORMATION_LEFT = 22;
constexpr int16_t FORMATION_TOP = 20;
constexpr int16_t FORMATION_STEP_X = 24;
constexpr int16_t FORMATION_STEP_Y = 19;
constexpr uint16_t JUMO_BLUE = 0x02D8;

int16_t pingPong(uint32_t elapsedMs, uint32_t durationMs, int16_t low, int16_t high)
{
    const uint32_t halfDurationMs = durationMs / 2U;
    const uint32_t phase = elapsedMs % durationMs;
    if (phase < halfDurationMs)
    {
        return static_cast<int16_t>(low + ((high - low) * phase) / halfDurationMs);
    }
    return static_cast<int16_t>(high - ((high - low) * (phase - halfDurationMs)) / halfDurationMs);
}

uint16_t spriteColor(uint8_t displayIdx, uint8_t spriteIdx)
{
    constexpr uint16_t colors[] = {ST77XX_CYAN, ST77XX_MAGENTA, ST77XX_YELLOW};
    return colors[(displayIdx + spriteIdx) % (sizeof(colors) / sizeof(colors[0]))];
}

void drawSignalSprite(Adafruit_ST7735& display, int16_t centerX, int16_t topY, uint16_t color)
{
    display.fillRect(centerX - 6, topY + 3, 13, 5, color);
    display.fillRect(centerX - 3, topY, 7, 3, color);
    display.fillRect(centerX - 9, topY + 6, 4, 3, color);
    display.fillRect(centerX + 6, topY + 6, 4, 3, color);
    display.drawPixel(centerX - 3, topY + 5, ST77XX_WHITE);
    display.drawPixel(centerX + 3, topY + 5, ST77XX_WHITE);
}

void drawDefender(Adafruit_ST7735& display, int16_t centerX)
{
    constexpr int16_t topY = 104;
    display.fillTriangle(centerX - 14, topY + 13, centerX + 14, topY + 13, centerX, topY, JUMO_BLUE);
    display.fillRect(centerX - 5, topY + 8, 11, 7, JUMO_BLUE);
    display.drawFastHLine(centerX - 11, topY + 13, 23, ST77XX_CYAN);
}

void drawExplosion(Adafruit_ST7735& display, int16_t centerX, int16_t centerY)
{
    display.drawFastHLine(centerX - 7, centerY, 15, ST77XX_WHITE);
    display.drawFastVLine(centerX, centerY - 7, 15, ST77XX_WHITE);
    display.drawLine(centerX - 5, centerY - 5, centerX + 5, centerY + 5, ST77XX_YELLOW);
    display.drawLine(centerX - 5, centerY + 5, centerX + 5, centerY - 5, ST77XX_YELLOW);
}
}  // namespace

void PixelParadeEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                 uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool PixelParadeEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void PixelParadeEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    const int16_t formationOffsetX = pingPong(elapsedMs, MARCH_CYCLE_DURATION_MS, 0, 18);
    const int16_t formationOffsetY = static_cast<int16_t>((elapsedMs / MARCH_CYCLE_DURATION_MS) % 3U) * 6;
    const int16_t defenderX = pingPong(elapsedMs + displayIdx * 220U, 2400, 28, DISP_WIDTH - 28);
    const uint32_t shotElapsedMs = elapsedMs % SHOT_CYCLE_DURATION_MS;
    const bool shotVisible = shotElapsedMs < 520U;
    const bool explosionVisible = shotElapsedMs >= 520U && shotElapsedMs < 720U;
    const uint8_t targetIndex = static_cast<uint8_t>((elapsedMs / SHOT_CYCLE_DURATION_MS + displayIdx) %
                                                     (FORMATION_COLUMNS * FORMATION_ROWS));

    display.fillScreen(ST77XX_BLACK);
    display.setTextColor(JUMO_BLUE);
    display.setTextSize(1);
    display.setCursor(4, 7);
    display.print("JUMO SIGNAL DEFENSE");
    display.drawFastHLine(4, 16, DISP_WIDTH - 8, JUMO_BLUE);

    for (uint8_t row = 0; row < FORMATION_ROWS; row++)
    {
        for (uint8_t column = 0; column < FORMATION_COLUMNS; column++)
        {
            const uint8_t spriteIndex = static_cast<uint8_t>(row * FORMATION_COLUMNS + column);
            const int16_t x = static_cast<int16_t>(FORMATION_LEFT + formationOffsetX + column * FORMATION_STEP_X);
            const int16_t y = static_cast<int16_t>(FORMATION_TOP + formationOffsetY + row * FORMATION_STEP_Y);
            if (explosionVisible && spriteIndex == targetIndex)
            {
                drawExplosion(display, x, static_cast<int16_t>(y + 5));
            }
            else
            {
                drawSignalSprite(display, x, y, spriteColor(displayIdx, spriteIndex));
            }
        }
    }

    if (shotVisible)
    {
        const int16_t laserTop = static_cast<int16_t>(104 - (shotElapsedMs * 72U) / 520U);
        display.drawFastVLine(defenderX, laserTop, 13, ST77XX_CYAN);
    }
    drawDefender(display, defenderX);
}