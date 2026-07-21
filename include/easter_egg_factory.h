// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

class EasterEgg;

class EasterEggFactory
{
   public:
    static EasterEgg* create(uint16_t eggId);
};