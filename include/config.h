// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

// ── I2C Bus ───────────────────────────────────────────────────────────────────
constexpr uint8_t I2C_SDA_PIN = 4;
constexpr uint8_t I2C_SCL_PIN = 5;

// ── SSD1306 Displays ──────────────────────────────────────────────────────────
constexpr uint8_t DISP1_I2C_ADDR = 0x3C;
constexpr uint8_t DISP2_I2C_ADDR = 0x3D;
constexpr uint8_t DISP_WIDTH = 128;
constexpr uint8_t DISP_HEIGHT = 64;

// ── Motor GPIO ────────────────────────────────────────────────────────────────
constexpr uint8_t MOTOR_PIN = 15;

// ── Modbus ────────────────────────────────────────────────────────────────────
constexpr uint8_t MODBUS_UNIT_ID = 1;
constexpr uint32_t MODBUS_BAUD = 9600;
constexpr uint8_t MODBUS_NUM_COILS = 1;
constexpr uint8_t MODBUS_NUM_REGS = 20;  // 16 display + 3 version + 1 temperature

// Coil indices
constexpr uint8_t COIL_MOTOR = 0;

// Holding register base offsets (4 regs × 2 chars = 8 chars per line)
constexpr uint8_t REG_DISP1_LINE1 = 0;
constexpr uint8_t REG_DISP1_LINE2 = 4;
constexpr uint8_t REG_DISP2_LINE1 = 8;
constexpr uint8_t REG_DISP2_LINE2 = 12;

// Version registers (FC03, read-only by convention).
// Each holds one numeric component of the firmware version tag (e.g. "01.02.03" → 1, 2, 3).
constexpr uint8_t REG_VERSION_MAJOR = 16;
constexpr uint8_t REG_VERSION_MINOR = 17;
constexpr uint8_t REG_VERSION_PATCH = 18;

// Temperature register (FC03 read / FC06 write, display 1).
// Value = temperature × 10  (e.g. 235 → 23.5 °C).
// Write 0xFFFF to disable temperature mode and restore text display.
constexpr uint8_t  REG_TEMPERATURE   = 19;
constexpr uint16_t TEMP_REG_DISABLED = 0xFFFFU;
