// SPDX-License-Identifier: GPL-3.0-or-later
#include "easter_egg_factory.h"

#include "config.h"
#include "debug_console_easter_egg.h"
#include "matrix_rain_easter_egg.h"
#include "melting_text_easter_egg.h"
#include "modbus_packet_easter_egg.h"
#include "oscilloscope_easter_egg.h"
#include "pixel_parade_easter_egg.h"
#include "temperature_rocket_easter_egg.h"
#include "tetris_easter_egg.h"

namespace
{
MeltingTextEasterEgg s_meltingTextEasterEgg;
TetrisEasterEgg s_tetrisEasterEgg;
TemperatureRocketEasterEgg s_temperatureRocketEasterEgg;
ModbusPacketEasterEgg s_modbusPacketEasterEgg;
OscilloscopeEasterEgg s_oscilloscopeEasterEgg;
MatrixRainEasterEgg s_matrixRainEasterEgg;
DebugConsoleEasterEgg s_debugConsoleEasterEgg;
PixelParadeEasterEgg s_pixelParadeEasterEgg;
}

EasterEgg* EasterEggFactory::create(uint16_t eggId)
{
    if (eggId == EASTER_EGG_MELTING_TEXT)
    {
        return &s_meltingTextEasterEgg;
    }
    if (eggId == EASTER_EGG_TETRIS)
    {
        return &s_tetrisEasterEgg;
    }
    if (eggId == EASTER_EGG_TEMPERATURE_ROCKET)
    {
        return &s_temperatureRocketEasterEgg;
    }
    if (eggId == EASTER_EGG_MODBUS_PACKET)
    {
        return &s_modbusPacketEasterEgg;
    }
    if (eggId == EASTER_EGG_OSCILLOSCOPE)
    {
        return &s_oscilloscopeEasterEgg;
    }
    if (eggId == EASTER_EGG_MATRIX_RAIN)
    {
        return &s_matrixRainEasterEgg;
    }
    if (eggId == EASTER_EGG_DEBUG_CONSOLE)
    {
        return &s_debugConsoleEasterEgg;
    }
    if (eggId == EASTER_EGG_PIXEL_PARADE)
    {
        return &s_pixelParadeEasterEgg;
    }

    return nullptr;
}