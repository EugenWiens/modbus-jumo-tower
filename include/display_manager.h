// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

#include "config.h"
#include "easter_egg.h"

class DisplayManager
{
   public:
    void init();

    // idx: 0 = Display 1, 1 = Display 2, 2 = Display 3
    // line1/line2: null-terminated strings, max 8 chars each
    void update(uint8_t idx, const char* line1, const char* line2);

    // Shows two lines centred in large text with the specified text size.
    void showLargeText(uint8_t idx, uint8_t textSize, const char* line1, const char* line2);

    // Shows temperature in °C centred on the display in large font.
    // Replaces normal text mode; call update() to return to text mode.
    void showTemperature(uint8_t idx, float tempC);

    // Shows temperature above relative humidity. Temperature uses the larger font.
    void showClimate(uint8_t idx, float tempC, float humidityPercent);

    // Starts the requested animation on all displays. Returns false for unknown IDs.
    bool startEasterEgg(uint16_t eggId, uint32_t nowMs);
    // Renders the next animation frame when necessary and restores the base view after completion.
    void updateEasterEgg(uint32_t nowMs);

   private:
    void setBaseSnapshot(uint8_t idx, const EasterEggDisplaySnapshot& snapshot);
    void renderSnapshot(uint8_t idx, const EasterEggDisplaySnapshot& snapshot) const;

    EasterEggDisplaySnapshot _baseSnapshots[MAX_DISPLAY_COUNT] = {};
    EasterEgg* _activeEasterEgg = nullptr;
    uint32_t _lastEasterEggFrameMs = 0;
};
