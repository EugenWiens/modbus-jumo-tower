# modbus-jumo-tower

Firmware for a **Raspberry Pi Pico (RP2040)** that acts as a Modbus RTU slave over USB CDC-ACM.
It drives two SH1106 OLED displays and a motor output, and can display a temperature value sent by the Modbus master.

The USB device enumerates as **JUMO / JUMO Tower**.

## Hardware

| Component | Detail |
|-----------|--------|
| MCU | Raspberry Pi Pico (RP2040) |
| Display 1 | SH1106 128×64 OLED, I²C address `0x3C` |
| Display 2 | SH1106 128×64 OLED, I²C address `0x3D` |
| I²C bus | SDA = GPIO 4, SCL = GPIO 5 |
| Motor output | GPIO 16 (digital on/off) |
| Modbus transport | USB CDC-ACM (`/dev/ttyACM0`), 115200 baud |
| Debug output | Second USB CDC-ACM interface (`/dev/ttyACM1`), 115200 baud |

## Wiring

### Schematic

```
                                    Raspberry Pi Pico
                                   ┌─────────────────┐
                       USB (PC) ───┤ USB             │
                                   │                 │
             3V3 rail ◄────────────┤ 3V3  (pin 36)   │
             GND rail ◄────────────┤ GND  (pin 38)   │
                                   │                 │
  SDA ────◄──────────────────────◄─┤ GP4  (pin  6)   │
  SCL ────◄──────────────────────◄─┤ GP5  (pin  7)   │
                                   │                 │
  Motor ──◄──────────────────────◄─┤ GP16 (pin 21)   │
                                   └─────────────────┘

  SDA ───┬───────────────────────────────────────┐
  SCL ───┼──┬────────────────────────────────┐   │
         │  │                                │   │
  ┌──────┴──┴──────────┐              ┌──────┴───┴──────────┐
  │      SH1106 #1     │              │      SH1106 #2       │
  ├────────────────────┤              ├─────────────────────┤
  │ SDA                │              │ SDA                 │
  │ SCL                │              │ SCL                 │
  │ VCC ◄── 3V3 rail   │              │ VCC ◄── 3V3 rail    │
  │ GND ──► GND rail   │              │ GND ──► GND rail    │
  │ SA0 ──► GND (0x3C) │              │ SA0 ──► 3V3  (0x3D) │
  └────────────────────┘              └─────────────────────┘

  Motor ──► [1 kΩ] ──► Base / Gate
                        │  NPN / N-MOSFET / Relay module
                        ├── Collector / Drain ──► Motor (+)
                        └── Emitter  / Source ──► GND
                            (1N4007 flyback diode across motor terminals)
```

### Pin assignment

| Pico pin | GPIO | Signal | Connects to |
|----------|------|--------|-------------|
| 6 | GP4 | I²C SDA | SDA on both SH1106 modules |
| 7 | GP5 | I²C SCL | SCL on both SH1106 modules |
| 21 | GP16 | Motor OUT | Base/Gate of driver transistor / relay IN |
| 36 | 3V3 | Power | VCC on both SH1106 modules |
| 38 | GND | Ground | GND on all peripherals |

### Notes

- **I²C address selection:** SH1106 address is set by the `SA0` pad — tie to **GND** for `0x3C`, to **3V3** for `0x3D`.
- **Pull-up resistors:** most SH1106 breakout boards include 4.7 kΩ pull-ups on SDA/SCL. If using bare modules, add them externally.
- **Motor driver:** GP16 is a 3.3 V logic output. Use an NPN transistor, N-MOSFET, or a relay module with built-in driver. Add a **flyback diode** (e.g. 1N4007) across inductive loads.
- **Power:** the Pico's 3V3 rail can supply ≈ 300 mA. For larger loads, supply SH1106 VCC separately from VSYS/VBUS.

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

Writing a value other than `0xFFFF` to register `19` switches **both displays** into temperature mode: the numeric value is rendered as `XX.XC` and all display text registers (`0`–`15`) are ignored until temperature mode is disabled again by writing `0xFFFF`.

## Debug output

The second USB CDC-ACM interface (`/dev/ttyACM1`) provides startup, I²C-scan, display-update, motor-state, and temperature-mode messages at 115200 baud. It is separate from the Modbus interface on `/dev/ttyACM0`.

## License

GPL-3.0-or-later — see [LICENSE](LICENSE).

