# modbus-jumo-tower

Firmware for a **Raspberry Pi Pico (RP2040)** that acts as a Modbus RTU slave over USB CDC-ACM.
It drives three ST7735 TFT displays and a motor output, and can display a temperature value sent by the Modbus master.

The USB device enumerates as **JUMO / JUMO Tower**.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | Raspberry Pi Pico (RP2040) |
| Displays | 3 x ST7735 TFT, 128x160 pixels, shared SPI bus |
| SPI bus | SCK = GPIO 18, MOSI = GPIO 19 |
| Display control | CS1 = GPIO 13, CS2 = GPIO 14, CS3 = GPIO 15, DC = GPIO 20, Reset = GPIO 21 |
| Motor output | GPIO 16 (digital on/off) |
| Modbus transport | USB CDC-ACM (`/dev/ttyACM0`), 115200 baud |
| Debug output | Second USB CDC-ACM interface (`/dev/ttyACM1`), 115200 baud |

## Wiring

### Schematic

```
Raspberry Pi Pico               ST7735 #1      ST7735 #2      ST7735 #3
-----------------               ---------      ---------      ---------
GP13 (pin 17) ----------------- CS
GP14 (pin 19) --------------------------------- CS
GP15 (pin 20) ------------------------------------------------ CS
GP18 (pin 24) ----------------- SCK ----------- SCK ----------- SCK
GP19 (pin 25) ----------------- MOSI/SDA ------ MOSI/SDA ------ MOSI/SDA
GP20 (pin 26) ----------------- DC/A0 --------- DC/A0 --------- DC/A0
GP21 (pin 27) ----------------- RST/RES ------- RST/RES ------- RST/RES
3V3  (pin 36) ----------------- VCC ----------- VCC ----------- VCC
GND  (pin 38) ----------------- GND ----------- GND ----------- GND

GP16 (pin 21) ----------------- motor driver input
```

### Pin assignment

| Pico pin | GPIO | Signal | Connects to |
|----------|------|--------|-------------|
| 17 | GP13 | TFT CS1 | CS on ST7735 #1 |
| 19 | GP14 | TFT CS2 | CS on ST7735 #2 |
| 20 | GP15 | TFT CS3 | CS on ST7735 #3 |
| 21 | GP16 | Motor OUT | Base/Gate of driver transistor / relay IN |
| 24 | GP18 | SPI SCK | SCK on all ST7735 modules |
| 25 | GP19 | SPI MOSI | MOSI/SDA on all ST7735 modules |
| 26 | GP20 | TFT DC | DC/A0 on all ST7735 modules |
| 27 | GP21 | TFT Reset | RST/RES on all ST7735 modules |
| 36 | 3V3 | Power | VCC on the ST7735 modules |
| 38 | GND | Ground | GND on all peripherals |

### Notes

- **SPI connections:** connect `SCK`, `MOSI` (sometimes labelled `SDA`), `DC` (sometimes `A0`) and `RST`/`RES` to all three modules. Connect each module's `CS` only to its assigned Pico GPIO. `MISO` is not required.
- **Display variant:** the firmware uses the `INITR_BLACKTAB` initialization for common 1.8-inch 128x160 modules. If the visible image is shifted, adjust `ST7735_INIT_OPTION` in [include/config.h](include/config.h) for the tab variant of the module.
- **Motor driver:** GP16 is a 3.3 V logic output. Use an NPN transistor, N-MOSFET, or a relay module with built-in driver. Add a **flyback diode** (e.g. 1N4007) across inductive loads.
- **Power:** three backlit TFT modules can exceed the current available from the Pico's 3V3 rail. Use a sufficiently rated 3.3 V supply when needed and always connect its ground to Pico GND.

## Build

Built with [PlatformIO](https://platformio.org/) using the Earle Philhower arduino-pico core.

```
pio run          # compile
pio run -t upload  # flash
```

The firmware version is injected automatically from the latest Git tag via `get_version.py`.

## Modbus Interface

- **Unit ID:** `1`
- **Baud rate:** `115200` (USB CDC-ACM – baud is advisory but required by the library)
- **Parity / stop bits:** library default (none / 1)

### Coils — FC01 read / FC05 write

| Address | Name | Description |
|---------|------|-------------|
| `0` | `COIL_MOTOR` | `1` = motor on, `0` = motor off |

### Holding Registers — FC03 read / FC06 write

| Address | Name | Type | Description |
|---------|------|------|-------------|
| `0`–`3` | `REG_DISP1_LINE1` | R/W | Display 1, line 1 — 8 ASCII chars packed 2 per register¹ |
| `4`–`7` | `REG_DISP1_LINE2` | R/W | Display 1, line 2 — 8 ASCII chars packed 2 per register¹ |
| `8`–`11` | `REG_DISP2_LINE1` | R/W | Display 2, line 1 — 8 ASCII chars packed 2 per register¹ |
| `12`–`15` | `REG_DISP2_LINE2` | R/W | Display 2, line 2 — 8 ASCII chars packed 2 per register¹ |
| `16`–`19` | `REG_DISP3_LINE1` | R/W | Display 3, line 1 — 8 ASCII chars packed 2 per register¹ |
| `20`–`23` | `REG_DISP3_LINE2` | R/W | Display 3, line 2 — 8 ASCII chars packed 2 per register¹ |
| `24` | `REG_TEMPERATURE` | R/W | Temperature x 10 in C (e.g. `235` -> 23.5 C). Write `0xFFFF` to disable and restore text on all displays. |
| `25` | `REG_VERSION_MAJOR` | R | Firmware version — major component |
| `26` | `REG_VERSION_MINOR` | R | Firmware version — minor component |
| `27` | `REG_VERSION_PATCH` | R | Firmware version — patch component |

¹ **Text encoding:** each register holds two ASCII characters — high byte is the first character, low byte is the second. Four consecutive registers form one 8-character display line. Write null bytes (`0x00`) to pad shorter strings.

### Temperature mode

Writing a value other than `0xFFFF` to register `24` switches **all three displays** into temperature mode: the numeric value is rendered as `XX.XC` and all display text registers (`0`–`23`) are ignored until temperature mode is disabled again by writing `0xFFFF`.

## Debug output

The second USB CDC-ACM interface (`/dev/ttyACM1`) provides startup, display-update, motor-state, and temperature-mode messages at 115200 baud. It is separate from the Modbus interface on `/dev/ttyACM0`.

## License

GPL-3.0-or-later — see [LICENSE](LICENSE).

