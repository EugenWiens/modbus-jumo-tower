// SPDX-License-Identifier: GPL-3.0-or-later
#include "modbus_packet_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t PACKET_CYCLE_DURATION_MS = 5000;
constexpr uint32_t PACKET_TRAVEL_DURATION_MS = 1300;
constexpr uint32_t CONFIRMATION_DURATION_MS = 900;
constexpr int16_t PACKET_WIDTH = 42;
constexpr int16_t PACKET_HEIGHT = 20;
constexpr int16_t TRACK_Y = 67;

void drawTitle(Adafruit_ST7735& display, uint8_t displayIdx)
{
    display.setTextColor(ST77XX_CYAN);
    display.setTextSize(2);
    display.setCursor(44, 10);
    display.print("MODBUS");

    display.setTextColor(ST77XX_WHITE);
    display.setTextSize(1);
    display.setCursor(62, 33);
    display.print("NODE ");
    display.print(displayIdx + 1U);
}

void drawTrack(Adafruit_ST7735& display)
{
    for (int16_t x = 10; x < DISP_WIDTH - 10; x += 10)
    {
        display.drawFastHLine(x, TRACK_Y + PACKET_HEIGHT / 2, 5, ST77XX_BLUE);
    }
}

void drawPacket(Adafruit_ST7735& display, int16_t x, uint16_t color)
{
    display.fillRoundRect(x, TRACK_Y, PACKET_WIDTH, PACKET_HEIGHT, 3, color);
    display.drawRoundRect(x, TRACK_Y, PACKET_WIDTH, PACKET_HEIGHT, 3, ST77XX_WHITE);
    display.setTextColor(ST77XX_BLACK);
    display.setTextSize(1);
    display.setCursor(static_cast<int16_t>(x + 5), static_cast<int16_t>(TRACK_Y + 6));
    display.print("01 03");
}

void drawStatus(Adafruit_ST7735& display, const char* status, uint16_t color)
{
    display.setTextColor(color);
    display.setTextSize(2);
    display.setCursor(static_cast<int16_t>((DISP_WIDTH - 6 * strlen(status) * 2) / 2), 103);
    display.print(status);
}
}  // namespace

void ModbusPacketEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                                  uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool ModbusPacketEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void ModbusPacketEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t cycleElapsedMs = (nowMs - _startMs) % PACKET_CYCLE_DURATION_MS;
    const uint32_t stageStartMs = displayIdx * PACKET_TRAVEL_DURATION_MS;
    const uint32_t confirmationStartMs = 3U * PACKET_TRAVEL_DURATION_MS;
    const uint16_t packetColor = displayIdx == 0U ? ST77XX_GREEN
                                 : displayIdx == 1U ? ST77XX_YELLOW
                                                    : ST77XX_MAGENTA;

    display.fillScreen(ST77XX_BLACK);
    drawTitle(display, displayIdx);
    drawTrack(display);

    if (cycleElapsedMs >= stageStartMs && cycleElapsedMs < stageStartMs + PACKET_TRAVEL_DURATION_MS)
    {
        const uint32_t stageElapsedMs = cycleElapsedMs - stageStartMs;
        const int16_t x = static_cast<int16_t>(-PACKET_WIDTH +
                                               ((DISP_WIDTH + PACKET_WIDTH) * stageElapsedMs) /
                                                   PACKET_TRAVEL_DURATION_MS);
        drawPacket(display, x, packetColor);
        drawStatus(display, displayIdx == 0U ? "TX" : "RX", packetColor);
        return;
    }

    if (cycleElapsedMs >= confirmationStartMs &&
        cycleElapsedMs < confirmationStartMs + CONFIRMATION_DURATION_MS)
    {
        drawStatus(display, "CRC OK", ST77XX_GREEN);
        display.fillCircle(DISP_WIDTH / 2, TRACK_Y + PACKET_HEIGHT / 2, 14, ST77XX_GREEN);
        display.setTextColor(ST77XX_BLACK);
        display.setTextSize(3);
        display.setCursor(72, 61);
        display.print("+");
        return;
    }

    drawStatus(display, displayIdx == 0U ? "READY" : "WAIT", ST77XX_BLUE);
}