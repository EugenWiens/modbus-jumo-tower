// SPDX-License-Identifier: GPL-3.0-or-later
#include "oscilloscope_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t SWEEP_PIXEL_DURATION_MS = 18;
constexpr int16_t PLOT_TOP = 29;
constexpr int16_t PLOT_BOTTOM = 108;
constexpr int16_t PLOT_CENTER_Y = (PLOT_TOP + PLOT_BOTTOM) / 2;

int16_t signalY(uint16_t sample, uint8_t displayIdx)
{
    constexpr uint16_t signalPeriod = 64;
    const uint16_t phase = (sample + displayIdx * 21U) % signalPeriod;
    const int16_t triangle = phase <= signalPeriod / 2U ? static_cast<int16_t>(phase)
                                                         : static_cast<int16_t>(signalPeriod - phase);
    int16_t y = static_cast<int16_t>(PLOT_CENTER_Y + (triangle - 16) * 2);
    if (phase == 45U)
    {
        y = static_cast<int16_t>(PLOT_TOP + 3);
    }
    else if (phase == 46U)
    {
        y = static_cast<int16_t>(PLOT_BOTTOM - 3);
    }
    return y;
}

uint16_t signalColor(uint8_t displayIdx)
{
    constexpr uint16_t colors[] = {ST77XX_GREEN, ST77XX_CYAN, ST77XX_YELLOW};
    return colors[displayIdx % (sizeof(colors) / sizeof(colors[0]))];
}

void drawGrid(Adafruit_ST7735& display)
{
    for (int16_t x = 0; x < DISP_WIDTH; x += 20)
    {
        display.drawFastVLine(x, PLOT_TOP, PLOT_BOTTOM - PLOT_TOP + 1, ST77XX_BLUE);
    }
    for (int16_t y = PLOT_TOP; y <= PLOT_BOTTOM; y += 20)
    {
        display.drawFastHLine(0, y, DISP_WIDTH, ST77XX_BLUE);
    }
    display.drawFastHLine(0, PLOT_CENTER_Y, DISP_WIDTH, ST77XX_MAGENTA);
}
}  // namespace

void OscilloscopeEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                  uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool OscilloscopeEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void OscilloscopeEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    const int16_t sweepX = static_cast<int16_t>((elapsedMs / SWEEP_PIXEL_DURATION_MS) % DISP_WIDTH);
    const uint16_t color = signalColor(displayIdx);

    display.fillScreen(ST77XX_BLACK);
    display.setTextColor(ST77XX_WHITE);
    display.setTextSize(1);
    display.setCursor(4, 8);
    display.print("SCOPE ");
    display.print(displayIdx + 1U);
    display.setTextColor(color);
    display.setCursor(111, 8);
    display.print("TRIG");
    drawGrid(display);

    for (int16_t x = 1; x <= sweepX; x++)
    {
        const uint16_t sample = static_cast<uint16_t>((elapsedMs / 7U) + x);
        display.drawLine(static_cast<int16_t>(x - 1), signalY(sample - 1U, displayIdx), x,
                         signalY(sample, displayIdx), color);
    }

    display.drawFastVLine(sweepX, PLOT_TOP, PLOT_BOTTOM - PLOT_TOP + 1, ST77XX_RED);
}