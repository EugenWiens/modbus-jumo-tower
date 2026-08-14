// SPDX-License-Identifier: GPL-3.0-or-later
#include "rotating_jumo_logo_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"
#include "jumo_logo.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t ROTATION_DURATION_MS = 4000;
constexpr float FULL_ROTATION_RADIANS = 6.28318530718F;
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
    const float angle = ((nowMs - _startMs) % ROTATION_DURATION_MS) * FULL_ROTATION_RADIANS /
                        ROTATION_DURATION_MS;
    drawJumoLogo(display, DISP_WIDTH / 2, DISP_HEIGHT / 2, angle);
}