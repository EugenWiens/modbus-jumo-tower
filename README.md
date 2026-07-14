# modbus-jumo-tower

Firmware for a **Raspberry Pi Pico (RP2040)** that acts as a Modbus RTU slave over USB CDC-ACM.
It drives two SSD1306 OLED displays and a motor output, and can display a temperature value sent by the Modbus master.

The USB device enumerates as **JUMO / JUMO Tower**.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | Raspberry Pi Pico (RP2040) |
| Display 1 | SSD1306 128×64 OLED, I²C address `0x3C` |
| Display 2 | SSD1306 128×64 OLED, I²C address `0x3D` |
| I²C bus | SDA = GPIO 4, SCL = GPIO 5 |
| Motor output | GPIO 15 (digital on/off) |
| Modbus transport | USB CDC-ACM (`/dev/ttyACM0`), 115200 baud |

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
| `16` | `REG_VERSION_MAJOR` | R | Firmware version — major component |
| `17` | `REG_VERSION_MINOR` | R | Firmware version — minor component |
| `18` | `REG_VERSION_PATCH` | R | Firmware version — patch component |
| `19` | `REG_TEMPERATURE` | R/W | Temperature × 10 in °C (e.g. `235` → 23.5 °C). Write `0xFFFF` to disable and restore text on both displays. |

¹ **Text encoding:** each register holds two ASCII characters — high byte is the first character, low byte is the second. Four consecutive registers form one 8-character display line. Write null bytes (`0x00`) to pad shorter strings.

### Temperature mode

Writing a value other than `0xFFFF` to register `19` switches **both displays** into temperature mode: the numeric value is rendered as `XX.X °C` and all display text registers (`0`–`15`) are ignored until temperature mode is disabled again by writing `0xFFFF`.

## License

GPL-3.0-or-later — see [LICENSE](LICENSE).

