// SPDX-License-Identifier: GPL-3.0-or-later
#include "rotating_jumo_logo_easter_egg.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <math.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t ROTATION_DURATION_MS = 4000;
constexpr int16_t LOGO_WIDTH = 120;
constexpr int16_t LOGO_HEIGHT = 48;
constexpr uint16_t JUMO_BLUE = 0x02D8;
constexpr float FULL_ROTATION_RADIANS = 6.28318530718F;

void drawLogo(Adafruit_ST7735& display, uint32_t elapsedMs)
{
    GFXcanvas1 logo(LOGO_WIDTH, LOGO_HEIGHT);
    logo.fillScreen(0);
    logo.drawRoundRect(0, 0, LOGO_WIDTH, LOGO_HEIGHT, 20, 1);
    logo.drawRoundRect(2, 2, LOGO_WIDTH - 4, LOGO_HEIGHT - 4, 18, 1);
    logo.setTextColor(1);
    logo.setTextSize(4);
    logo.setCursor(12, 8);
    logo.print("JUMO");

    const float angle = (elapsedMs % ROTATION_DURATION_MS) * FULL_ROTATION_RADIANS / ROTATION_DURATION_MS;
    const float cosine = cosf(angle);
    const float sine = sinf(angle);
    const float centerX = (LOGO_WIDTH - 1) / 2.0F;
    const float centerY = (LOGO_HEIGHT - 1) / 2.0F;

    for (int16_t y = 0; y < LOGO_HEIGHT; y++)
    {
        for (int16_t x = 0; x < LOGO_WIDTH; x++)
        {
            if (!logo.getPixel(x, y))
            {
                continue;
            }

            const float relativeX = x - centerX;
            const float relativeY = y - centerY;
            const int16_t rotatedX = static_cast<int16_t>(lroundf(relativeX * cosine - relativeY * sine)) +
                                     DISP_WIDTH / 2;
            const int16_t rotatedY = static_cast<int16_t>(lroundf(relativeX * sine + relativeY * cosine)) +
                                     DISP_HEIGHT / 2;
            display.drawPixel(rotatedX, rotatedY, JUMO_BLUE);
        }
    }
}
}  // namespace

void RotatingJumoLogoEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                      uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool RotatingJumoLogoEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void RotatingJumoLogoEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display,
                                             uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    display.fillScreen(ST77XX_WHITE);
    drawLogo(display, nowMs - _startMs);
}