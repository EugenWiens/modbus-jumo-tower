// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

class DisplayManager
{
 public:
  void init(uint8_t addr1, uint8_t addr2);

  // idx: 0 = Display 1 (0x3C), 1 = Display 2 (0x3D)
  // line1/line2: null-terminated strings, max 8 chars each
  void update(uint8_t idx, const char* line1, const char* line2);
};
