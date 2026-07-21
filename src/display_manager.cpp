// SPDX-License-Identifier: GPL-3.0-or-later
#include "display_manager.h"

#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <stdio.h>

#include "config.h"
#include "easter_egg_factory.h"

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
    delay(50);
    digitalWrite(DISP_RST_PIN, LOW);
    delay(50);
    digitalWrite(DISP_RST_PIN, HIGH);
    delay(50);

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

    EasterEggDisplaySnapshot snapshot = {};
    snprintf(snapshot.lines[0].text, sizeof(snapshot.lines[0].text), "%s", line1);
    snprintf(snapshot.lines[1].text, sizeof(snapshot.lines[1].text), "%s", line2);
    snapshot.lines[0].textSizeX = 2;
    snapshot.lines[0].textSizeY = 2;
    snapshot.lines[1].textSizeX = 2;
    snapshot.lines[1].textSizeY = 2;

    int16_t bx1, by1, bx2, by2;
    uint16_t bw1, bh1, bw2, bh2;
    s_disp[idx].setTextSize(snapshot.lines[0].textSizeX, snapshot.lines[0].textSizeY);
    s_disp[idx].getTextBounds(line1, 0, 0, &bx1, &by1, &bw1, &bh1);
    s_disp[idx].getTextBounds(line2, 0, 0, &bx2, &by2, &bw2, &bh2);

    constexpr int16_t lineGap = 8;
    const int16_t textTop = static_cast<int16_t>((s_disp[idx].height() - bh1 - lineGap - bh2) / 2);
    snapshot.lines[0].x = static_cast<int16_t>((s_disp[idx].width() - bw1) / 2 - bx1);
    snapshot.lines[0].y = static_cast<int16_t>(textTop - by1);
    snapshot.lines[1].x = static_cast<int16_t>((s_disp[idx].width() - bw2) / 2 - bx2);
    snapshot.lines[1].y = static_cast<int16_t>(textTop + bh1 + lineGap - by2);
    setBaseSnapshot(idx, snapshot);
}

void DisplayManager::showLargeText(uint8_t idx, uint8_t textSize, const char* line1, const char* line2)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    EasterEggDisplaySnapshot snapshot = {};
    snprintf(snapshot.lines[0].text, sizeof(snapshot.lines[0].text), "%s", line1);
    snprintf(snapshot.lines[1].text, sizeof(snapshot.lines[1].text), "%s", line2);
    snapshot.lines[0].textSizeX = textSize;
    snapshot.lines[0].textSizeY = textSize;
    snapshot.lines[1].textSizeX = textSize;
    snapshot.lines[1].textSizeY = textSize;

    int16_t bx1, by1, bx2, by2;
    uint16_t bw1, bh1, bw2, bh2;
    s_disp[idx].setTextSize(textSize);
    s_disp[idx].getTextBounds(line1, 0, 0, &bx1, &by1, &bw1, &bh1);
    s_disp[idx].getTextBounds(line2, 0, 0, &bx2, &by2, &bw2, &bh2);

    constexpr int16_t lineGap = 8;
    const int16_t textTop = static_cast<int16_t>((s_disp[idx].height() - bh1 - lineGap - bh2) / 2);
    snapshot.lines[0].x = static_cast<int16_t>((s_disp[idx].width() - bw1) / 2 - bx1);
    snapshot.lines[0].y = static_cast<int16_t>(textTop - by1);
    snapshot.lines[1].x = static_cast<int16_t>((s_disp[idx].width() - bw2) / 2 - bx2);
    snapshot.lines[1].y = static_cast<int16_t>(textTop + bh1 + lineGap - by2);
    setBaseSnapshot(idx, snapshot);
}

void DisplayManager::showTemperature(uint8_t idx, float tempC)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    char buf[12];
    snprintf(buf, sizeof(buf), tempC < 0.0F ? "%.0fC" : "%.1fC", static_cast<double>(tempC));

    EasterEggDisplaySnapshot snapshot = {};
    snprintf(snapshot.lines[0].text, sizeof(snapshot.lines[0].text), "%s", buf);
    snapshot.lines[0].textSizeX = 5;
    snapshot.lines[0].textSizeY = 5;

    int16_t bx, by;
    uint16_t bw, bh;
    s_disp[idx].setTextSize(snapshot.lines[0].textSizeX, snapshot.lines[0].textSizeY);
    s_disp[idx].getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    snapshot.lines[0].x = static_cast<int16_t>((DISP_WIDTH - bw) / 2);
    snapshot.lines[0].y = static_cast<int16_t>((DISP_HEIGHT - bh) / 2 - by);
    setBaseSnapshot(idx, snapshot);
}

void DisplayManager::showClimate(uint8_t idx, float tempC, float humidityPercent)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    char temperatureBuf[12];
    char humidityBuf[12];
    snprintf(temperatureBuf, sizeof(temperatureBuf), tempC < 0.0F ? "%.0fC" : "%.1fC",
             static_cast<double>(tempC));
    snprintf(humidityBuf, sizeof(humidityBuf), "%.1f%%", static_cast<double>(humidityPercent));

    EasterEggDisplaySnapshot snapshot = {};
    snprintf(snapshot.lines[0].text, sizeof(snapshot.lines[0].text), "%s", temperatureBuf);
    snprintf(snapshot.lines[1].text, sizeof(snapshot.lines[1].text), "%s", humidityBuf);

    uint8_t temperatureTextWidth = 5;
    constexpr uint8_t temperatureTextHeight = 7;
    s_disp[idx].setTextSize(temperatureTextWidth, temperatureTextHeight);
    int16_t temperatureBx, temperatureBy;
    uint16_t temperatureBw, temperatureBh;
    s_disp[idx].getTextBounds(temperatureBuf, 0, 0, &temperatureBx, &temperatureBy,
                              &temperatureBw, &temperatureBh);
    if (temperatureBw > s_disp[idx].width())
    {
        temperatureTextWidth = 4;
        s_disp[idx].setTextSize(temperatureTextWidth, temperatureTextHeight);
        s_disp[idx].getTextBounds(temperatureBuf, 0, 0, &temperatureBx, &temperatureBy,
                                  &temperatureBw, &temperatureBh);
    }

    constexpr uint8_t humidityTextWidth = 3;
    constexpr uint8_t humidityTextHeight = 5;
    s_disp[idx].setTextSize(humidityTextWidth, humidityTextHeight);
    int16_t humidityBx, humidityBy;
    uint16_t humidityBw, humidityBh;
    s_disp[idx].getTextBounds(humidityBuf, 0, 0, &humidityBx, &humidityBy, &humidityBw, &humidityBh);

    constexpr int16_t temperatureTop = 8;
    constexpr int16_t humidityBottomMargin = 10;
    const int16_t humidityTop = static_cast<int16_t>(s_disp[idx].height() - humidityBh - humidityBottomMargin);
    snapshot.lines[0].textSizeX = temperatureTextWidth;
    snapshot.lines[0].textSizeY = temperatureTextHeight;
    snapshot.lines[0].x = static_cast<int16_t>((s_disp[idx].width() - temperatureBw) / 2 - temperatureBx);
    snapshot.lines[0].y = static_cast<int16_t>(temperatureTop - temperatureBy);
    snapshot.lines[1].textSizeX = humidityTextWidth;
    snapshot.lines[1].textSizeY = humidityTextHeight;
    snapshot.lines[1].x = static_cast<int16_t>((s_disp[idx].width() - humidityBw) / 2 - humidityBx);
    snapshot.lines[1].y = static_cast<int16_t>(humidityTop - humidityBy);
    setBaseSnapshot(idx, snapshot);
}

bool DisplayManager::startEasterEgg(uint16_t eggId, uint32_t nowMs)
{
    EasterEgg* easterEgg = EasterEggFactory::create(eggId);
    if (easterEgg == nullptr)
    {
        return false;
    }

    _activeEasterEgg = easterEgg;
    _activeEasterEgg->start(nowMs, _baseSnapshots, DISPLAY_COUNT);
    _lastEasterEggFrameMs = nowMs;
    for (uint8_t displayIdx = 0; displayIdx < DISPLAY_COUNT; displayIdx++)
    {
        _activeEasterEgg->renderFrame(displayIdx, s_disp[displayIdx], nowMs);
    }
    return true;
}

void DisplayManager::updateEasterEgg(uint32_t nowMs)
{
    if (_activeEasterEgg == nullptr)
    {
        return;
    }

    if (_activeEasterEgg->isFinished(nowMs))
    {
        _activeEasterEgg = nullptr;
        for (uint8_t displayIdx = 0; displayIdx < DISPLAY_COUNT; displayIdx++)
        {
            renderSnapshot(displayIdx, _baseSnapshots[displayIdx]);
        }
        return;
    }

    constexpr uint32_t frameIntervalMs = 50;
    if (nowMs - _lastEasterEggFrameMs < frameIntervalMs)
    {
        return;
    }

    _lastEasterEggFrameMs = nowMs;
    for (uint8_t displayIdx = 0; displayIdx < DISPLAY_COUNT; displayIdx++)
    {
        _activeEasterEgg->renderFrame(displayIdx, s_disp[displayIdx], nowMs);
    }
}

void DisplayManager::setBaseSnapshot(uint8_t idx, const EasterEggDisplaySnapshot& snapshot)
{
    _baseSnapshots[idx] = snapshot;
    if (_activeEasterEgg == nullptr)
    {
        renderSnapshot(idx, snapshot);
    }
}

void DisplayManager::renderSnapshot(uint8_t idx, const EasterEggDisplaySnapshot& snapshot) const
{
    s_disp[idx].fillScreen(ST77XX_BLACK);
    s_disp[idx].setTextColor(ST77XX_WHITE);

    for (uint8_t lineIdx = 0; lineIdx < EASTER_EGG_MAX_LINES; lineIdx++)
    {
        const EasterEggTextLine& line = snapshot.lines[lineIdx];
        s_disp[idx].setTextSize(line.textSizeX, line.textSizeY);
        s_disp[idx].setCursor(line.x, line.y);
        s_disp[idx].print(line.text);
    }
}
