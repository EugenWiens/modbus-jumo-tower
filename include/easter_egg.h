// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

class Adafruit_ST7735;

constexpr uint8_t EASTER_EGG_MAX_LINES = 2;
constexpr uint8_t EASTER_EGG_MAX_TEXT_LENGTH = 12;

struct EasterEggTextLine
{
    char text[EASTER_EGG_MAX_TEXT_LENGTH];
    uint8_t textSizeX;
    uint8_t textSizeY;
    int16_t x;
    int16_t y;
};

struct EasterEggDisplaySnapshot
{
    EasterEggTextLine lines[EASTER_EGG_MAX_LINES];
};

class EasterEgg
{
   public:
    virtual ~EasterEgg() = default;

    virtual void start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                       uint8_t displayCount) = 0;
    virtual bool isFinished(uint32_t nowMs) const = 0;
    virtual void renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const = 0;
};