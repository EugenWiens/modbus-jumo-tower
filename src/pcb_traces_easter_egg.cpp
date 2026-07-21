// SPDX-License-Identifier: GPL-3.0-or-later
#include "pcb_traces_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t TRACE_GROW_DURATION_MS = 450;
constexpr uint32_t PULSE_DURATION_MS = 700;
constexpr uint16_t JUMO_BLUE = 0x02D8;
constexpr int16_t CHIP_LEFT = 57;
constexpr int16_t CHIP_TOP = 49;
constexpr int16_t CHIP_WIDTH = 46;
constexpr int16_t CHIP_HEIGHT = 31;

struct TraceSegment
{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
};

constexpr TraceSegment TRACE_SEGMENTS[] = {
    {10, 25, 43, 25}, {43, 25, 43, 49}, {43, 49, CHIP_LEFT, 49},
    {150, 25, 117, 25}, {117, 25, 117, 49}, {117, 49, CHIP_LEFT + CHIP_WIDTH, 49},
    {10, 103, 43, 103}, {43, 103, 43, 80}, {43, 80, CHIP_LEFT, 80},
    {150, 103, 117, 103}, {117, 103, 117, 80}, {117, 80, CHIP_LEFT + CHIP_WIDTH, 80},
};
constexpr uint8_t TRACE_COUNT = sizeof(TRACE_SEGMENTS) / sizeof(TRACE_SEGMENTS[0]);

void drawSegment(Adafruit_ST7735& display, const TraceSegment& segment, uint16_t progress, uint16_t color)
{
    const int16_t endX = static_cast<int16_t>(segment.x1 + ((segment.x2 - segment.x1) * progress) / 1000);
    const int16_t endY = static_cast<int16_t>(segment.y1 + ((segment.y2 - segment.y1) * progress) / 1000);
    display.drawLine(segment.x1, segment.y1, endX, endY, color);
}

void drawChip(Adafruit_ST7735& display, bool isConnected)
{
    const uint16_t outlineColor = isConnected ? ST77XX_CYAN : JUMO_BLUE;
    display.fillRoundRect(CHIP_LEFT, CHIP_TOP, CHIP_WIDTH, CHIP_HEIGHT, 3, ST77XX_BLACK);
    display.drawRoundRect(CHIP_LEFT, CHIP_TOP, CHIP_WIDTH, CHIP_HEIGHT, 3, outlineColor);
    display.setTextColor(JUMO_BLUE);
    display.setTextSize(2);
    display.setCursor(62, 58);
    display.print("JUMO");

    for (int16_t pin = 0; pin < 4; pin++)
    {
        const int16_t pinY = static_cast<int16_t>(55 + pin * 7);
        display.drawFastHLine(CHIP_LEFT - 5, pinY, 5, outlineColor);
        display.drawFastHLine(CHIP_LEFT + CHIP_WIDTH, pinY, 5, outlineColor);
    }
}

void drawNode(Adafruit_ST7735& display, int16_t x, int16_t y, uint16_t color)
{
    display.fillCircle(x, y, 3, color);
    display.drawCircle(x, y, 4, ST77XX_WHITE);
}
}  // namespace

void PcbTracesEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool PcbTracesEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void PcbTracesEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    const uint32_t traceBuildDurationMs = TRACE_COUNT * TRACE_GROW_DURATION_MS;
    const bool isConnected = elapsedMs >= traceBuildDurationMs;

    display.fillScreen(ST77XX_BLACK);
    display.setTextSize(1);
    display.setTextColor(JUMO_BLUE);
    display.setCursor(46, 7);
    display.print(isConnected ? "PCB CONNECTED" : "PCB LINKING");

    for (uint8_t traceIdx = 0; traceIdx < TRACE_COUNT; traceIdx++)
    {
        const uint32_t traceStartMs = traceIdx * TRACE_GROW_DURATION_MS;
        uint16_t progress = 0;
        if (elapsedMs >= traceStartMs + TRACE_GROW_DURATION_MS)
        {
            progress = 1000;
        }
        else if (elapsedMs > traceStartMs)
        {
            progress = static_cast<uint16_t>(((elapsedMs - traceStartMs) * 1000U) /
                                              TRACE_GROW_DURATION_MS);
        }
        drawSegment(display, TRACE_SEGMENTS[traceIdx], progress, JUMO_BLUE);
    }

    drawNode(display, 10, 25, isConnected ? ST77XX_CYAN : JUMO_BLUE);
    drawNode(display, 150, 25, isConnected ? ST77XX_CYAN : JUMO_BLUE);
    drawNode(display, 10, 103, isConnected ? ST77XX_CYAN : JUMO_BLUE);
    drawNode(display, 150, 103, isConnected ? ST77XX_CYAN : JUMO_BLUE);
    drawChip(display, isConnected);

    if (isConnected)
    {
        const uint32_t pulseElapsedMs = elapsedMs - traceBuildDurationMs + displayIdx * 170U;
        const uint8_t pulseTraceIdx = static_cast<uint8_t>((pulseElapsedMs / PULSE_DURATION_MS) % TRACE_COUNT);
        const uint16_t pulseProgress = static_cast<uint16_t>((pulseElapsedMs % PULSE_DURATION_MS) * 1000U /
                                                              PULSE_DURATION_MS);
        const TraceSegment& pulseTrace = TRACE_SEGMENTS[pulseTraceIdx];
        const int16_t pulseX = static_cast<int16_t>(pulseTrace.x1 +
                                                    ((pulseTrace.x2 - pulseTrace.x1) * pulseProgress) / 1000);
        const int16_t pulseY = static_cast<int16_t>(pulseTrace.y1 +
                                                    ((pulseTrace.y2 - pulseTrace.y1) * pulseProgress) / 1000);
        display.fillCircle(pulseX, pulseY, 3, ST77XX_WHITE);
    }
}