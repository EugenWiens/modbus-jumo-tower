// SPDX-License-Identifier: GPL-3.0-or-later
#include "jumo_logo.h"

#include <math.h>

namespace
{
constexpr int16_t LOGO_WIDTH = 120;
constexpr int16_t LOGO_HEIGHT = 48;
constexpr uint16_t JUMO_BLUE = 0x02D8;
}

void drawJumoLogo(Adafruit_GFX& display, int16_t centerX, int16_t centerY, float rotationRadians)
{
    GFXcanvas1 logo(LOGO_WIDTH, LOGO_HEIGHT);
    logo.fillScreen(0);
    logo.drawRoundRect(0, 0, LOGO_WIDTH, LOGO_HEIGHT, 20, 1);
    logo.drawRoundRect(2, 2, LOGO_WIDTH - 4, LOGO_HEIGHT - 4, 18, 1);
    logo.setTextColor(1);
    logo.setTextSize(4);
    logo.setCursor(12, 8);
    logo.print("JUMO");

    const float cosine = cosf(rotationRadians);
    const float sine = sinf(rotationRadians);
    const float logoCenterX = (LOGO_WIDTH - 1) / 2.0F;
    const float logoCenterY = (LOGO_HEIGHT - 1) / 2.0F;

    for (int16_t y = 0; y < LOGO_HEIGHT; y++)
    {
        for (int16_t x = 0; x < LOGO_WIDTH; x++)
        {
            if (!logo.getPixel(x, y))
            {
                continue;
            }

            const float relativeX = x - logoCenterX;
            const float relativeY = y - logoCenterY;
            const int16_t rotatedX = static_cast<int16_t>(lroundf(relativeX * cosine - relativeY * sine)) +
                                     centerX;
            const int16_t rotatedY = static_cast<int16_t>(lroundf(relativeX * sine + relativeY * cosine)) +
                                     centerY;
            display.drawPixel(rotatedX, rotatedY, JUMO_BLUE);
        }
    }
}