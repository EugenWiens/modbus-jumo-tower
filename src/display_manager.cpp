// SPDX-License-Identifier: GPL-3.0-or-later
#include "display_manager.h"

#include <Adafruit_SH110X.h>
#include <Wire.h>
#include <stdio.h>

#include "config.h"

static Adafruit_SH1106G s_disp[2] = {
    Adafruit_SH1106G(DISP_WIDTH, DISP_HEIGHT, &Wire, -1),
    Adafruit_SH1106G(DISP_WIDTH, DISP_HEIGHT, &Wire, -1),
};

void DisplayManager::init(const uint8_t (&addrs)[2])
{
    for (uint8_t i = 0; i < 2; i++)
    {
        s_disp[i].begin(addrs[i], false);
        s_disp[i].clearDisplay();
        s_disp[i].display();
    }
}

void DisplayManager::update(uint8_t idx, const char* line1, const char* line2)
{
    if (idx > 1)
    {
        return;
    }

    s_disp[idx].clearDisplay();
    s_disp[idx].setTextSize(1);
    s_disp[idx].setTextColor(SH110X_WHITE);
    s_disp[idx].setCursor(0, 0);
    s_disp[idx].print(line1);
    s_disp[idx].setCursor(0, 16);
    s_disp[idx].print(line2);
    s_disp[idx].display();
}

void DisplayManager::showLargeText(uint8_t idx, uint8_t textSize, const char* line1, const char* line2)
{
    if (idx > 1)
    {
        return;
    }

    s_disp[idx].clearDisplay();
    s_disp[idx].setTextColor(SH110X_WHITE);
    s_disp[idx].setTextSize(textSize);

    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].getTextBounds(line1, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2), 8);
    s_disp[idx].print(line1);

    s_disp[idx].getTextBounds(line2, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2), 36);
    s_disp[idx].print(line2);
    s_disp[idx].display();
}

void DisplayManager::showTemperature(uint8_t idx, float tempC)
{
    if (idx > 1)
    {
        return;
    }

    // Format as "xx.x\xB0C" (degree symbol + C)
    char buf[12];
    snprintf(buf, sizeof(buf), "%.1fC", static_cast<double>(tempC));

    s_disp[idx].clearDisplay();
    s_disp[idx].setTextColor(SH110X_WHITE);
    s_disp[idx].setTextSize(4);

    // Centre horizontally and vertically using measured text bounds
    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2),
                          static_cast<int16_t>((DISP_HEIGHT - bh) / 2));
    s_disp[idx].print(buf);
    s_disp[idx].display();
}
