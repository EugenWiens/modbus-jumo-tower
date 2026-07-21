// SPDX-License-Identifier: GPL-3.0-or-later
#include "easter_egg_factory.h"

#include "config.h"
#include "melting_text_easter_egg.h"
#include "tetris_easter_egg.h"

namespace
{
MeltingTextEasterEgg s_meltingTextEasterEgg;
TetrisEasterEgg s_tetrisEasterEgg;
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

    return nullptr;
}