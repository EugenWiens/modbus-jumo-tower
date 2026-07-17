// SPDX-License-Identifier: GPL-3.0-or-later
#include "modbus_handler.h"

#include <string.h>

ModbusHandler::ModbusHandler() : _modbus(Serial)
{
    memset(coils, 0, sizeof(coils));
    memset(holdingRegs, 0, sizeof(holdingRegs));
    holdingRegs[REG_TEMPERATURE] = TEMP_REG_DISABLED;  // disabled until master writes a value
}

void ModbusHandler::begin()
{
    Serial.begin(MODBUS_BAUD);  // USB CDC-ACM – baud is advisory for USB but required by the library
    _modbus.configureCoils(coils, MODBUS_NUM_COILS);
    _modbus.configureHoldingRegisters(holdingRegs, MODBUS_NUM_REGS);
    _modbus.begin(MODBUS_UNIT_ID, MODBUS_BAUD);

    prepareVersionString();
}

void ModbusHandler::prepareVersionString()
{
    // Store each version component as a separate uint16_t register.
    // Tag "01.02.03" → REG_VERSION_MAJOR=1, REG_VERSION_MINOR=2, REG_VERSION_PATCH=3.
    holdingRegs[REG_VERSION_MAJOR] = static_cast<uint16_t>(FIRMWARE_VERSION_MAJOR);
    holdingRegs[REG_VERSION_MINOR] = static_cast<uint16_t>(FIRMWARE_VERSION_MINOR);
    holdingRegs[REG_VERSION_PATCH] = static_cast<uint16_t>(FIRMWARE_VERSION_PATCH);
}

void ModbusHandler::poll()
{
    _modbus.poll();
}

bool ModbusHandler::getMotorState() const
{
    return coils[COIL_MOTOR];
}

bool ModbusHandler::hasTemperature() const
{
    return holdingRegs[REG_TEMPERATURE] != TEMP_REG_DISABLED;
}

float ModbusHandler::getTemperature() const
{
    return static_cast<float>(holdingRegs[REG_TEMPERATURE]) / 10.0f;
}

void ModbusHandler::getDisplayText(uint8_t dispIdx, uint8_t lineIdx, char* buf) const
{
    // Register layout: [D1L1:0-3][D1L2:4-7][D2L1:8-11][D2L2:12-15][D3L1:16-19][D3L2:20-23]
    // Each register encodes 2 ASCII chars: high byte first, then low byte.
    const uint8_t baseIdx = static_cast<uint8_t>(DISPLAY_REG_BASES[dispIdx] + (lineIdx * 4U));
    for (uint8_t i = 0; i < 4; i++)
    {
        buf[2 * i] = static_cast<char>((holdingRegs[baseIdx + i] >> 8) & 0xFFU);
        buf[2 * i + 1] = static_cast<char>(holdingRegs[baseIdx + i] & 0xFFU);
    }
    buf[8] = '\0';
}
