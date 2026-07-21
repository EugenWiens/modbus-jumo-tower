// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <stdint.h>

// ── ST7735 Displays (shared SPI bus) ──────────────────────────────────────────
constexpr uint8_t DISPLAY_COUNT = 3;
constexpr uint8_t MAX_DISPLAY_COUNT = 3;  // for array sizing, not used in code
constexpr uint8_t DISP_WIDTH = 160;
constexpr uint8_t DISP_HEIGHT = 128;
constexpr uint8_t DISP_ROTATION = 1;  // 0 = portrait, 1 = landscape, 2 = portrait flipped, 3 = landscape flipped
constexpr uint8_t SPI_SCK_PIN = 18;
constexpr uint8_t SPI_MOSI_PIN = 19;
constexpr uint8_t DISP_CS_PINS[MAX_DISPLAY_COUNT] = {13, 14, 15};
constexpr uint8_t DISP_DC_PIN = 20;
constexpr uint8_t DISP_RST_PIN = 21;
constexpr uint8_t ST7735_INIT_OPTION = 0x00;  // INITR_GREENTAB (colstart=2, rowstart=1 fixes pixel-snow on ST7735S clones)
// ── Motor GPIO ────────────────────────────────────────────────────────────────
constexpr uint8_t MOTOR_PIN = 16;

// ── Modbus ────────────────────────────────────────────────────────────────────
constexpr uint8_t MODBUS_UNIT_ID = 1;
constexpr uint32_t MODBUS_BAUD = 115200;
constexpr uint8_t MODBUS_NUM_COILS = 1;
constexpr uint8_t MODBUS_NUM_REGS = 32;  // 24 display + 2 temperature + 2 humidity + 3 version + 1 Easter Egg

// Coil indices
constexpr uint8_t COIL_MOTOR = 0;

// Display text (4 regs x 2 chars = 8 chars per line)
constexpr uint8_t REG_DISP1_LINE1 = 0;
constexpr uint8_t REG_DISP1_LINE2 = 4;
constexpr uint8_t REG_DISP2_LINE1 = 8;
constexpr uint8_t REG_DISP2_LINE2 = 12;
constexpr uint8_t REG_DISP3_LINE1 = 16;
constexpr uint8_t REG_DISP3_LINE2 = 20;
constexpr uint8_t DISPLAY_REG_BASES[MAX_DISPLAY_COUNT] = {
	REG_DISP1_LINE1,
	REG_DISP2_LINE1,
	REG_DISP3_LINE1,
};

// Climate display mode (FC03 read / FC16 write).
// IEEE-754 floats in two registers each, high word first.
// Write 0xFFFF to both registers of the temperature value to disable climate mode
// and restore text display.
constexpr uint8_t REG_TEMPERATURE_HIGH = 24;
constexpr uint8_t REG_TEMPERATURE_LOW = 25;
constexpr uint8_t REG_HUMIDITY_HIGH = 26;
constexpr uint8_t REG_HUMIDITY_LOW = 27;
constexpr uint16_t TEMP_REG_DISABLED = 0xFFFFU;

// Version registers (FC03, read-only by convention).
// Each holds one numeric component of the firmware version tag (e.g. "01.02.03" → 1, 2, 3).
constexpr uint8_t REG_VERSION_MAJOR = 28;
constexpr uint8_t REG_VERSION_MINOR = 29;
constexpr uint8_t REG_VERSION_PATCH = 30;

// Easter Egg command register (FC03 read / FC06 or FC16 write).
// The firmware resets a non-zero command to 0 after processing it.
constexpr uint8_t REG_EASTER_EGG = 31;
constexpr uint16_t EASTER_EGG_NONE = 0;
constexpr uint16_t EASTER_EGG_MELTING_TEXT = 1;
constexpr uint16_t EASTER_EGG_TETRIS = 2;

// ── Debug Serial (2nd USB CDC, /dev/ttyACM1) ──────────────────────────────────
#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
extern Adafruit_USBD_CDC DBG_SERIAL;
#endif
