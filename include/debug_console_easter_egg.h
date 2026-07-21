// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "easter_egg.h"

class DebugConsoleEasterEgg final : public EasterEgg
{
   public:
    void start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots, uint8_t displayCount) override;
    bool isFinished(uint32_t nowMs) const override;
    void renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const override;

   private:
    uint32_t _startMs = 0;
    uint8_t _displayCount = 0;
};