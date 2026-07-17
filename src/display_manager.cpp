// SPDX-License-Identifier: GPL-3.0-or-later
#include "display_manager.h"

#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <stdio.h>

#include "config.h"

static Adafruit_ST7735 s_disp[DISPLAY_COUNT] = {
    Adafruit_ST7735(DISP_CS_PINS[0], DISP_DC_PIN, DISP_RST_PIN),
    Adafruit_ST7735(DISP_CS_PINS[1], DISP_DC_PIN, DISP_RST_PIN),
    Adafruit_ST7735(DISP_CS_PINS[2], DISP_DC_PIN, DISP_RST_PIN),
};

void DisplayManager::init()
{
    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        pinMode(DISP_CS_PINS[i], OUTPUT);
        digitalWrite(DISP_CS_PINS[i], HIGH);
    }

    SPI.setSCK(SPI_SCK_PIN);
    SPI.setTX(SPI_MOSI_PIN);
    SPI.begin();

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        s_disp[i].initR(ST7735_INIT_OPTION);
        s_disp[i].setRotation(0);
        s_disp[i].fillScreen(ST77XX_BLACK);
    }
}

void DisplayManager::update(uint8_t idx, const char* line1, const char* line2)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    s_disp[idx].fillScreen(ST77XX_BLACK);
    s_disp[idx].setTextSize(2);
    s_disp[idx].setTextColor(ST77XX_WHITE);
    s_disp[idx].setCursor(8, 48);
    s_disp[idx].print(line1);
    s_disp[idx].setCursor(8, 96);
    s_disp[idx].print(line2);
}

void DisplayManager::showLargeText(uint8_t idx, uint8_t textSize, const char* line1, const char* line2)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    s_disp[idx].fillScreen(ST77XX_BLACK);
    s_disp[idx].setTextColor(ST77XX_WHITE);
    s_disp[idx].setTextSize(textSize);

    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].getTextBounds(line1, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2), 48);
    s_disp[idx].print(line1);

    s_disp[idx].getTextBounds(line2, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2), 96);
    s_disp[idx].print(line2);
}

void DisplayManager::showTemperature(uint8_t idx, float tempC)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    char buf[12];
    snprintf(buf, sizeof(buf), "%.1fC", static_cast<double>(tempC));

    s_disp[idx].fillScreen(ST77XX_BLACK);
    s_disp[idx].setTextColor(ST77XX_WHITE);
    s_disp[idx].setTextSize(4);

    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2),
                          static_cast<int16_t>((DISP_HEIGHT - bh) / 2 - by));
    s_disp[idx].print(buf);
}
