// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <ModbusRTUSlave.h>

#include "config.h"

class ModbusHandler
{
   public:
    // Public arrays written directly by the Modbus stack on every write command
    bool coils[MODBUS_NUM_COILS];
    uint16_t holdingRegs[MODBUS_NUM_REGS];

    ModbusHandler();
    void begin();
    void prepareVersionString();
    void poll();

    bool getMotorState() const;

    // Returns true when the temperature registers hold a finite IEEE-754 float.
    bool hasTemperature() const;
    // Temperature in °C. Call only when hasTemperature() is true.
    float getTemperature() const;

    // Decodes 4 holding registers into an 8-char string + null terminator.
    // dispIdx: 0, 1, or 2  |  lineIdx: 0 (line 1) or 1 (line 2)
    // buf must be at least 9 bytes.
    void getDisplayText(uint8_t dispIdx, uint8_t lineIdx, char* buf) const;

   private:
    ModbusRTUSlave _modbus;
};
