// SPDX-License-Identifier: GPL-3.0-or-later
#include "display_manager.h"

#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "easter_egg_factory.h"
#include "jumo_logo.h"

static Adafruit_ST7735 s_disp[MAX_DISPLAY_COUNT] = {
    Adafruit_ST7735(DISP_CS_PINS[0], DISP_DC_PIN, -1),
    Adafruit_ST7735(DISP_CS_PINS[1], DISP_DC_PIN, -1),
    Adafruit_ST7735(DISP_CS_PINS[2], DISP_DC_PIN, -1),
};

struct TextBounds
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
    bool valid;
};

static bool equalTextLine(const EasterEggTextLine& first, const EasterEggTextLine& second)
{
    return strcmp(first.text, second.text) == 0 && first.textSizeX == second.textSizeX &&
           first.textSizeY == second.textSizeY && first.x == second.x && first.y == second.y;
}

static bool equalSnapshot(const EasterEggDisplaySnapshot& first,
                          const EasterEggDisplaySnapshot& second)
{
    for (uint8_t lineIdx = 0; lineIdx < EASTER_EGG_MAX_LINES; lineIdx++)
    {
        if (!equalTextLine(first.lines[lineIdx], second.lines[lineIdx]))
        {
            return false;
        }
    }
    return true;
}

static TextBounds getTextBounds(Adafruit_ST7735& display, const EasterEggTextLine& line)
{
    if (line.text[0] == '\0')
    {
        return {0, 0, 0, 0, false};
    }

    display.setTextSize(line.textSizeX, line.textSizeY);
    int16_t x1;
    int16_t y1;
    uint16_t width;
    uint16_t height;
    display.getTextBounds(line.text, line.x, line.y, &x1, &y1, &width, &height);
    return {x1, y1, static_cast<int32_t>(x1) + width, static_cast<int32_t>(y1) + height, true};
}

static void clearTextBounds(Adafruit_ST7735& display, const TextBounds& first,
                            const TextBounds& second)
{
    if (!first.valid && !second.valid)
    {
        return;
    }

    int32_t left = first.valid ? first.left : second.left;
    int32_t top = first.valid ? first.top : second.top;
    int32_t right = first.valid ? first.right : second.right;
    int32_t bottom = first.valid ? first.bottom : second.bottom;
    if (second.valid)
    {
        if (second.left < left)
        {
            left = second.left;
        }
        if (second.top < top)
        {
            top = second.top;
        }
        if (second.right > right)
        {
            right = second.right;
        }
        if (second.bottom > bottom)
        {
            bottom = second.bottom;
        }
    }

    if (left < 0)
    {
        left = 0;
    }
    if (top < 0)
    {
        top = 0;
    }
    if (right > display.width())
    {
        right = display.width();
    }
    if (bottom > display.height())
    {
        bottom = display.height();
    }
    if (right <= left || bottom <= top)
    {
        return;
    }

    display.fillRect(static_cast<int16_t>(left), static_cast<int16_t>(top),
                     static_cast<uint16_t>(right - left), static_cast<uint16_t>(bottom - top),
                     ST77XX_BLACK);
}

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
        _hasRenderedSnapshot[i] = false;
    }
}

void DisplayManager::showBootLogo(uint8_t idx)
{
    if (idx >= DISPLAY_COUNT)
    {
        return;
    }

    _hasRenderedSnapshot[idx] = false;
    s_disp[idx].fillScreen(ST77XX_BLACK);
    drawJumoLogo(s_disp[idx], static_cast<int16_t>(s_disp[idx].width() / 2),
                 static_cast<int16_t>(s_disp[idx].height() / 2), 0.0F);
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
            _renderedSnapshots[displayIdx] = _baseSnapshots[displayIdx];
            _hasRenderedSnapshot[displayIdx] = true;
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
    const bool needsRender = !_hasRenderedSnapshot[idx] ||
                             !equalSnapshot(_renderedSnapshots[idx], snapshot);
    _baseSnapshots[idx] = snapshot;
    if (_activeEasterEgg != nullptr || !needsRender)
    {
        return;
    }

    if (_hasRenderedSnapshot[idx])
    {
        renderSnapshotDifferential(idx, _renderedSnapshots[idx], snapshot);
    }
    else
    {
        renderSnapshot(idx, snapshot);
    }
    _renderedSnapshots[idx] = snapshot;
    _hasRenderedSnapshot[idx] = true;
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

void DisplayManager::renderSnapshotDifferential(
    uint8_t idx, const EasterEggDisplaySnapshot& previous,
    const EasterEggDisplaySnapshot& current) const
{
    s_disp[idx].setTextColor(ST77XX_WHITE);

    for (uint8_t lineIdx = 0; lineIdx < EASTER_EGG_MAX_LINES; lineIdx++)
    {
        const EasterEggTextLine& previousLine = previous.lines[lineIdx];
        const EasterEggTextLine& currentLine = current.lines[lineIdx];
        if (equalTextLine(previousLine, currentLine))
        {
            continue;
        }

        const TextBounds previousBounds = getTextBounds(s_disp[idx], previousLine);
        const TextBounds currentBounds = getTextBounds(s_disp[idx], currentLine);
        clearTextBounds(s_disp[idx], previousBounds, currentBounds);

        if (currentLine.text[0] != '\0')
        {
            s_disp[idx].setTextSize(currentLine.textSizeX, currentLine.textSizeY);
            s_disp[idx].setCursor(currentLine.x, currentLine.y);
            s_disp[idx].print(currentLine.text);
        }
    }
}
