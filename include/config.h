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
constexpr uint8_t MODBUS_NUM_REGS = 16;

// Coil indices
constexpr uint8_t COIL_MOTOR = 0;

// Holding register base offsets (4 regs × 2 chars = 8 chars per line)
constexpr uint8_t REG_DISP1_LINE1 = 0;
constexpr uint8_t REG_DISP1_LINE2 = 4;
constexpr uint8_t REG_DISP2_LINE1 = 8;
constexpr uint8_t REG_DISP2_LINE2 = 12;
