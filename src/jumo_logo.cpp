// SPDX-License-Identifier: GPL-3.0-or-later
#include "jumo_logo.h"

#include <math.h>

namespace
{
constexpr int16_t LOGO_WIDTH = 138;
constexpr int16_t LOGO_HEIGHT = 56;
constexpr int16_t LOGO_CORNER_RADIUS = 23;
constexpr int16_t LOGO_BORDER_WIDTH = 3;
constexpr uint8_t LOGO_TEXT_SIZE = 5;

constexpr uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
    return static_cast<uint16_t>(((red & 0xF8U) << 8) | ((green & 0xFCU) << 3) | (blue >> 3));
}

constexpr uint16_t swapRgb565Channels(uint16_t color)
{
    const uint16_t red = (color >> 11) & 0x1FU;
    const uint16_t green = (color >> 5) & 0x3FU;
    const uint16_t blue = color & 0x1FU;
    return static_cast<uint16_t>((blue << 11) | (green << 5) | red);
}

constexpr uint16_t JUMO_LOGO_BLUE = swapRgb565Channels(0x02D8);
constexpr uint16_t JUMO_LOGO_BACKGROUND = rgb565(0xF0, 0xF1, 0xF1);

void drawLogoMask(Adafruit_GFX& display, GFXcanvas1& logo, int16_t centerX, int16_t centerY,
                  float rotationRadians, uint16_t color)
{
    const float cosine = cosf(rotationRadians);
    const float sine = sinf(rotationRadians);
    const float logoCenterX = static_cast<float>(LOGO_WIDTH / 2);
    const float logoCenterY = static_cast<float>(LOGO_HEIGHT / 2);

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
            display.drawPixel(rotatedX, rotatedY, color);
        }
    }
}
}

void drawJumoLogo(Adafruit_GFX& display, int16_t centerX, int16_t centerY, float rotationRadians)
{
    GFXcanvas1 logoBackground(LOGO_WIDTH, LOGO_HEIGHT);
    logoBackground.fillScreen(0);
    logoBackground.fillRoundRect(0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO_CORNER_RADIUS, 1);

    GFXcanvas1 logoForeground(LOGO_WIDTH, LOGO_HEIGHT);
    logoForeground.fillScreen(0);
    logoForeground.fillRoundRect(0, 0, LOGO_WIDTH, LOGO_HEIGHT, LOGO_CORNER_RADIUS, 1);
    logoForeground.fillRoundRect(LOGO_BORDER_WIDTH, LOGO_BORDER_WIDTH,
                                 LOGO_WIDTH - 2 * LOGO_BORDER_WIDTH,
                                 LOGO_HEIGHT - 2 * LOGO_BORDER_WIDTH,
                                 LOGO_CORNER_RADIUS - LOGO_BORDER_WIDTH, 0);
    logoForeground.setTextColor(1);
    logoForeground.setTextSize(LOGO_TEXT_SIZE);
    logoForeground.setCursor(9, 9);
    logoForeground.print("JUMO");

    drawLogoMask(display, logoBackground, centerX, centerY, rotationRadians, JUMO_LOGO_BACKGROUND);
    drawLogoMask(display, logoForeground, centerX, centerY, rotationRadians, JUMO_LOGO_BLUE);
}