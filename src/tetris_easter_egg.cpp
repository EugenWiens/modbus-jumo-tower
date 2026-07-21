// SPDX-License-Identifier: GPL-3.0-or-later
#include "tetris_easter_egg.h"

#include <Adafruit_ST7735.h>

#include "config.h"

namespace
{
constexpr uint32_t EASTER_EGG_DURATION_MS = 10000;
constexpr uint32_t CYCLE_DURATION_MS = 2400;
constexpr uint32_t FALL_DURATION_MS = 1700;
constexpr uint32_t CLEAR_DURATION_MS = 400;
constexpr uint8_t BOARD_COLUMNS = 10;
constexpr uint8_t BOARD_ROWS = 16;
constexpr int16_t CELL_SIZE = 7;

void drawBlock(Adafruit_ST7735& display, int16_t boardX, int16_t boardY, uint16_t color)
{
    if (boardX < 0 || boardX >= BOARD_COLUMNS || boardY < 0 || boardY >= BOARD_ROWS)
    {
        return;
    }

    constexpr int16_t boardLeft = (DISP_WIDTH - BOARD_COLUMNS * CELL_SIZE) / 2;
    constexpr int16_t boardTop = (DISP_HEIGHT - BOARD_ROWS * CELL_SIZE) / 2;
    const int16_t x = static_cast<int16_t>(boardLeft + boardX * CELL_SIZE);
    const int16_t y = static_cast<int16_t>(boardTop + boardY * CELL_SIZE);
    display.fillRect(x, y, CELL_SIZE - 1, CELL_SIZE - 1, color);
    display.drawPixel(x, y, ST77XX_WHITE);
}

uint16_t settledBlockColor(uint8_t column, uint8_t row, uint8_t cycle)
{
    constexpr uint16_t colors[] = {ST77XX_BLUE, ST77XX_MAGENTA, ST77XX_RED, ST77XX_ORANGE,
                                   ST77XX_GREEN, ST77XX_CYAN};
    return colors[(column + row + cycle) % (sizeof(colors) / sizeof(colors[0]))];
}

uint16_t fallingBlockColor(uint8_t cycle)
{
    constexpr uint16_t colors[] = {ST77XX_CYAN, ST77XX_YELLOW, ST77XX_GREEN};
    return colors[cycle % (sizeof(colors) / sizeof(colors[0]))];
}
}  // namespace

void TetrisEasterEgg::start(uint32_t startMs, const EasterEggDisplaySnapshot* snapshots,
                             uint8_t displayCount)
{
    static_cast<void>(snapshots);
    _startMs = startMs;
    _displayCount = displayCount;
}

bool TetrisEasterEgg::isFinished(uint32_t nowMs) const
{
    return nowMs - _startMs >= EASTER_EGG_DURATION_MS;
}

void TetrisEasterEgg::renderFrame(uint8_t displayIdx, Adafruit_ST7735& display, uint32_t nowMs) const
{
    if (displayIdx >= _displayCount)
    {
        return;
    }

    const uint32_t elapsedMs = nowMs - _startMs;
    const uint32_t cycle = elapsedMs / CYCLE_DURATION_MS;
    const uint32_t cycleElapsedMs = elapsedMs % CYCLE_DURATION_MS;
    const uint8_t gapColumn = static_cast<uint8_t>(2U + (cycle % 3U) * 2U);
    const bool isClearing = cycleElapsedMs >= FALL_DURATION_MS;
    const bool showCompletedRows = isClearing &&
                                   ((cycleElapsedMs - FALL_DURATION_MS) / 100U) % 2U == 0U &&
                                   cycleElapsedMs < FALL_DURATION_MS + CLEAR_DURATION_MS;

    display.fillScreen(ST77XX_BLACK);

    constexpr int16_t boardLeft = (DISP_WIDTH - BOARD_COLUMNS * CELL_SIZE) / 2;
    constexpr int16_t boardTop = (DISP_HEIGHT - BOARD_ROWS * CELL_SIZE) / 2;
    display.drawRect(boardLeft - 1, boardTop - 1, BOARD_COLUMNS * CELL_SIZE + 1, BOARD_ROWS * CELL_SIZE + 1,
                     ST77XX_WHITE);

    for (uint8_t row = BOARD_ROWS - 4; row < BOARD_ROWS; row++)
    {
        for (uint8_t column = 0; column < BOARD_COLUMNS; column++)
        {
            if (column != gapColumn || showCompletedRows)
            {
                drawBlock(display, column, row,
                          showCompletedRows ? ST77XX_WHITE : settledBlockColor(column, row, cycle));
            }
        }
    }

    if (!isClearing)
    {
        const int16_t startRow = -4;
        constexpr int16_t targetRow = BOARD_ROWS - 4;
        const int16_t fallDistance = targetRow - startRow;
        const int16_t fallingRow = static_cast<int16_t>(startRow +
                                                         (fallDistance * cycleElapsedMs) / FALL_DURATION_MS);
        const uint16_t color = fallingBlockColor(cycle);
        for (uint8_t block = 0; block < 4; block++)
        {
            drawBlock(display, gapColumn, static_cast<int16_t>(fallingRow + block), color);
        }
    }
}