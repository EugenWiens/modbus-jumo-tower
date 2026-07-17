// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

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
};
