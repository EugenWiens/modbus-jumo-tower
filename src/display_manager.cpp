// SPDX-License-Identifier: GPL-3.0-or-later
#include "display_manager.h"

#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <stdio.h>

#include "config.h"

static Adafruit_ST7735 s_disp[MAX_DISPLAY_COUNT] = {
    Adafruit_ST7735(DISP_CS_PINS[0], DISP_DC_PIN, -1),
    Adafruit_ST7735(DISP_CS_PINS[1], DISP_DC_PIN, -1),
    Adafruit_ST7735(DISP_CS_PINS[2], DISP_DC_PIN, -1),
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

    pinMode(DISP_RST_PIN, OUTPUT);
    digitalWrite(DISP_RST_PIN, HIGH);
    delay(100);
    digitalWrite(DISP_RST_PIN, LOW);
    delay(100);
    digitalWrite(DISP_RST_PIN, HIGH);
    delay(200);

    for (uint8_t i = 0; i < DISPLAY_COUNT; i++)
    {
        s_disp[i].initR(ST7735_INIT_OPTION);
        s_disp[i].setRotation(DISP_ROTATION);
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

    int16_t bx1, by1, bx2, by2;
    uint16_t bw1, bh1, bw2, bh2;
    s_disp[idx].getTextBounds(line1, 0, 0, &bx1, &by1, &bw1, &bh1);
    s_disp[idx].getTextBounds(line2, 0, 0, &bx2, &by2, &bw2, &bh2);

    constexpr int16_t lineGap = 8;
    const int16_t textTop = static_cast<int16_t>((s_disp[idx].height() - bh1 - lineGap - bh2) / 2);
    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - bw1) / 2 - bx1),
                          static_cast<int16_t>(textTop - by1));
    s_disp[idx].print(line1);
    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - bw2) / 2 - bx2),
                          static_cast<int16_t>(textTop + bh1 + lineGap - by2));
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

    int16_t bx1, by1, bx2, by2;
    uint16_t bw1, bh1, bw2, bh2;
    s_disp[idx].getTextBounds(line1, 0, 0, &bx1, &by1, &bw1, &bh1);
    s_disp[idx].getTextBounds(line2, 0, 0, &bx2, &by2, &bw2, &bh2);

    constexpr int16_t lineGap = 8;
    const int16_t textTop = static_cast<int16_t>((s_disp[idx].height() - bh1 - lineGap - bh2) / 2);
    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - bw1) / 2 - bx1),
                          static_cast<int16_t>(textTop - by1));
    s_disp[idx].print(line1);

    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - bw2) / 2 - bx2),
                          static_cast<int16_t>(textTop + bh1 + lineGap - by2));
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
    s_disp[idx].setTextSize(5);

    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    s_disp[idx].setCursor(static_cast<int16_t>((DISP_WIDTH - bw) / 2),
                          static_cast<int16_t>((DISP_HEIGHT - bh) / 2 - by));
    s_disp[idx].print(buf);
}

void DisplayManager::showClimate(uint8_t idx, float tempC, float humidityPercent)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    char temperatureBuf[12];
    char humidityBuf[12];
    snprintf(temperatureBuf, sizeof(temperatureBuf), "%.1fC", static_cast<double>(tempC));
    snprintf(humidityBuf, sizeof(humidityBuf), "%.1f%%", static_cast<double>(humidityPercent));

    s_disp[idx].fillScreen(ST77XX_BLACK);
    s_disp[idx].setTextColor(ST77XX_WHITE);

    uint8_t temperatureTextSize = 5;
    s_disp[idx].setTextSize(temperatureTextSize);
    int16_t temperatureBx, temperatureBy;
    uint16_t temperatureBw, temperatureBh;
    s_disp[idx].getTextBounds(temperatureBuf, 0, 0, &temperatureBx, &temperatureBy,
                              &temperatureBw, &temperatureBh);
    if (temperatureBw > s_disp[idx].width())
    {
        temperatureTextSize = 4;
        s_disp[idx].setTextSize(temperatureTextSize);
        s_disp[idx].getTextBounds(temperatureBuf, 0, 0, &temperatureBx, &temperatureBy,
                                  &temperatureBw, &temperatureBh);
    }

    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - temperatureBw) / 2 - temperatureBx),
                          static_cast<int16_t>(12 - temperatureBy));
    s_disp[idx].print(temperatureBuf);

    s_disp[idx].setTextSize(3);
    int16_t humidityBx, humidityBy;
    uint16_t humidityBw, humidityBh;
    s_disp[idx].getTextBounds(humidityBuf, 0, 0, &humidityBx, &humidityBy, &humidityBw, &humidityBh);
    s_disp[idx].setCursor(static_cast<int16_t>((s_disp[idx].width() - humidityBw) / 2 - humidityBx),
                          static_cast<int16_t>(78 - humidityBy));
    s_disp[idx].print(humidityBuf);
}
