// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

// ── ST7735 Displays (shared SPI bus) ──────────────────────────────────────────
constexpr uint8_t DISPLAY_COUNT = 3;
constexpr uint8_t DISP_WIDTH = 128;
constexpr uint8_t DISP_HEIGHT = 160;
constexpr uint8_t SPI_SCK_PIN = 18;
constexpr uint8_t SPI_MOSI_PIN = 19;
constexpr uint8_t DISP_CS_PINS[DISPLAY_COUNT] = {13, 14, 15};
constexpr uint8_t DISP_DC_PIN = 20;
constexpr uint8_t DISP_RST_PIN = 21;
constexpr uint8_t ST7735_INIT_OPTION = 0x05;  // INITR_BLACKTAB

// ── Motor GPIO ────────────────────────────────────────────────────────────────
constexpr uint8_t MOTOR_PIN = 16;

// ── Modbus ────────────────────────────────────────────────────────────────────
constexpr uint8_t MODBUS_UNIT_ID = 1;
constexpr uint32_t MODBUS_BAUD = 115200;
constexpr uint8_t MODBUS_NUM_COILS = 1;
constexpr uint8_t MODBUS_NUM_REGS = 28;  // 24 display + 1 temperature + 3 version

// Coil indices
constexpr uint8_t COIL_MOTOR = 0;

// Display text (4 regs x 2 chars = 8 chars per line)
constexpr uint8_t REG_DISP1_LINE1 = 0;
constexpr uint8_t REG_DISP1_LINE2 = 4;
constexpr uint8_t REG_DISP2_LINE1 = 8;
constexpr uint8_t REG_DISP2_LINE2 = 12;
constexpr uint8_t REG_DISP3_LINE1 = 16;
constexpr uint8_t REG_DISP3_LINE2 = 20;
constexpr uint8_t DISPLAY_REG_BASES[DISPLAY_COUNT] = {
	REG_DISP1_LINE1,
	REG_DISP2_LINE1,
	REG_DISP3_LINE1,
};

// Temperature display mode (FC03 read / FC06 write).
// Value = temperature x 10 (e.g. 235 -> 23.5 C).
// Write 0xFFFF to disable temperature mode and restore text display.
constexpr uint8_t REG_TEMPERATURE = 24;
constexpr uint16_t TEMP_REG_DISABLED = 0xFFFFU;

// Version registers (FC03, read-only by convention).
// Each holds one numeric component of the firmware version tag (e.g. "01.02.03" → 1, 2, 3).
constexpr uint8_t REG_VERSION_MAJOR = 25;
constexpr uint8_t REG_VERSION_MINOR = 26;
constexpr uint8_t REG_VERSION_PATCH = 27;

// ── Debug Serial (2nd USB CDC, /dev/ttyACM1) ──────────────────────────────────
#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
extern Adafruit_USBD_CDC DBG_SERIAL;
#endif
