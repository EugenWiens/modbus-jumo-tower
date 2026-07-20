#!/usr/bin/env python3
"""Generate schematic.drawio.png – a clear connection diagram for the JUMO Tower."""

from PIL import Image, ImageDraw, ImageFont

# ── Fonts ────────────────────────────────────────────────────────────────────
FONT_REGULAR = "/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf"
FONT_BOLD    = "/usr/share/fonts/google-droid-sans-fonts/DroidSans-Bold.ttf"

def font(size, bold=False):
    path = FONT_BOLD if bold else FONT_REGULAR
    return ImageFont.truetype(path, size)

# ── Colours ──────────────────────────────────────────────────────────────────
BG           = (255, 255, 255)
BOX_PICO     = (210, 225, 245)
BOX_PICO_BD  = (60,  90,  160)
BOX_DISP     = (210, 240, 220)
BOX_DISP_BD  = (40,  130, 70)
BOX_MOTOR    = (255, 235, 200)
BOX_MOTOR_BD = (180, 100, 20)
BOX_NOTE     = (250, 245, 200)
BOX_NOTE_BD  = (160, 140, 40)

C_SCK   = (30,  90,  200)   # SPI clock
C_MOSI  = (0,   160, 100)   # SPI data
C_DC    = (140, 40,  140)   # data/command
C_RST   = (180, 60,  60)    # reset
C_PWR   = (200, 30,  30)    # VCC
C_GND   = (60,  60,  60)    # GND
C_CS1   = (30,  140, 190)   # chip-select 1
C_CS2   = (20,  100, 160)   # chip-select 2
C_CS3   = (10,  60,  130)   # chip-select 3
C_MOTOR = (180, 80,  0)     # motor signal
TXT     = (20,  20,  20)

W = 1800
H = 960
SCALE = 2          # final 2× upscaling for crispness

img = Image.new("RGB", (W * SCALE, H * SCALE), BG)
d   = ImageDraw.Draw(img)

def s(v):
    """Scale a coordinate or list/tuple of coordinates."""
    if isinstance(v, (list, tuple)):
        return type(v)(s(i) for i in v)
    return v * SCALE

def rect(x1, y1, x2, y2, fill, outline, radius=8):
    d.rounded_rectangle(
        [s(x1), s(y1), s(x2), s(y2)],
        radius=s(radius), fill=fill, outline=outline, width=s(2)
    )

def text(x, y, txt, sz=13, bold=False, colour=TXT, anchor="la"):
    d.text((s(x), s(y)), txt, fill=colour, font=font(s(sz), bold), anchor=anchor)

def hline(x1, x2, y, colour, width=2):
    d.line([(s(x1), s(y)), (s(x2), s(y))], fill=colour, width=s(width))

def vline(x, y1, y2, colour, width=2):
    d.line([(s(x), s(y1)), (s(x), s(y2))], fill=colour, width=s(width))

def arrow_h(x1, x2, y, colour, width=2):
    """Horizontal arrow from x1 to x2."""
    hline(x1, x2, y, colour, width)
    dx = 6 if x2 > x1 else -6
    d.polygon(
        [s((x2, y)), s((x2 - dx, y - 4)), s((x2 - dx, y + 4))],
        fill=colour,
    )

def label_h(x, y, txt, sz=11, colour=TXT, right=False):
    anchor = "ra" if right else "la"
    d.text((s(x), s(y) - s(sz) // 2 - s(2)), txt, fill=colour,
           font=font(s(sz)), anchor=anchor)

# ─────────────────────────────────────────────────────────────────────────────
# Layout constants
# ─────────────────────────────────────────────────────────────────────────────
PICO_X1, PICO_Y1, PICO_X2, PICO_Y2 = 30, 30, 280, 850

BUS_X1, BUS_X2 = 340, 560

DISP_X1, DISP_X2 = 640, 900
D1_Y1, D1_Y2 = 60,  240
D2_Y1, D2_Y2 = 330, 510
D3_Y1, D3_Y2 = 600, 780

MOT_X1, MOT_X2  = 980, 1200
MOT_Y1, MOT_Y2  = 60,  420

NOTE_X1, NOTE_X2 = 980, 1200
NOTE_Y1, NOTE_Y2 = 460, 580

# ─────────────────────────────────────────────────────────────────────────────
# Pico box
# ─────────────────────────────────────────────────────────────────────────────
rect(PICO_X1, PICO_Y1, PICO_X2, PICO_Y2, BOX_PICO, BOX_PICO_BD)
text((PICO_X1 + PICO_X2) // 2, PICO_Y1 + 16, "Raspberry Pi Pico",
     sz=14, bold=True, colour=BOX_PICO_BD, anchor="ma")
text((PICO_X1 + PICO_X2) // 2, PICO_Y1 + 32, "(RP2040)",
     sz=11, colour=BOX_PICO_BD, anchor="ma")

# Pin table
pins = [
    ("17", "GP13", "CS Display #1",  C_CS1),
    ("19", "GP14", "CS Display #2",  C_CS2),
    ("20", "GP15", "CS Display #3",  C_CS3),
    ("21", "GP16", "Motor output",   C_MOTOR),
    ("24", "GP18", "SPI SCK",        C_SCK),
    ("25", "GP19", "SPI MOSI/SDA",   C_MOSI),
    ("26", "GP20", "DC / A0",        C_DC),
    ("27", "GP21", "RST / RES",      C_RST),
    ("36", "3V3",  "Display supply", C_PWR),
    ("38", "GND",  "Common ground",  C_GND),
]

# column positions inside Pico box
PX_PIN  = PICO_X1 + 14
PX_GPIO = PICO_X1 + 50
PX_SIG  = PICO_X1 + 95

# header
y0 = PICO_Y1 + 60
text(PX_PIN,  y0, "Pin",  sz=11, bold=True)
text(PX_GPIO, y0, "GPIO", sz=11, bold=True)
text(PX_SIG,  y0, "Signal", sz=11, bold=True)

ROW_H = 70
for i, (pin, gpio, sig, col) in enumerate(pins):
    y = y0 + 30 + i * ROW_H
    text(PX_PIN,  y, pin,  sz=12, colour=TXT)
    text(PX_GPIO, y, gpio, sz=12, colour=col, bold=True)
    text(PX_SIG,  y, sig,  sz=11, colour=col)
    # dots on right edge showing the signal exits
    cx = PICO_X2
    d.ellipse([s(cx - 5), s(y - 4), s(cx + 5), s(y + 6)], fill=col)

# USB label
rect(PICO_X1 + 15, PICO_Y2 - 80, PICO_X2 - 15, PICO_Y2 - 15,
     (235, 235, 250), BOX_PICO_BD, radius=5)
text((PICO_X1 + PICO_X2) // 2, PICO_Y2 - 58,
     "USB CDC-ACM", sz=11, bold=True, colour=BOX_PICO_BD, anchor="ma")
text((PICO_X1 + PICO_X2) // 2, PICO_Y2 - 40,
     "Modbus RTU  /dev/ttyACM0", sz=10, colour=BOX_PICO_BD, anchor="ma")
text((PICO_X1 + PICO_X2) // 2, PICO_Y2 - 24,
     "Debug       /dev/ttyACM1", sz=10, colour=BOX_PICO_BD, anchor="ma")

# ─────────────────────────────────────────────────────────────────────────────
# Signal bus column – vertical trunk lines
# ─────────────────────────────────────────────────────────────────────────────
BUS_TOP = PICO_Y1 + 50
BUS_BOT = D3_Y2 + 20

# We draw 8 trunk lines side-by-side
signals = [
    ("SCK",  C_SCK),
    ("MOSI", C_MOSI),
    ("DC",   C_DC),
    ("RST",  C_RST),
    ("VCC",  C_PWR),
    ("GND",  C_GND),
    ("CS1",  C_CS1),
    ("CS2",  C_CS2),
]
TRUNK_X_START = BUS_X1 + 10
TRUNK_SEP     = 26

trunk_xs = {}
for i, (name, col) in enumerate(signals):
    tx = TRUNK_X_START + i * TRUNK_SEP
    trunk_xs[name] = tx
    vline(tx, BUS_TOP, BUS_BOT, col, width=2)
    text(tx, BUS_TOP - 18, name, sz=9, colour=col, anchor="ma")

# CS3 individual line (reaches only display 3)
tx3 = TRUNK_X_START + len(signals) * TRUNK_SEP
trunk_xs["CS3"] = tx3
vline(tx3, D3_Y1 + 20, D3_Y2 - 20, C_CS3, width=2)

# ─────────────────────────────────────────────────────────────────────────────
# Horizontal connections from Pico to trunks
# The dots are at PICO_X2; trunks are near BUS_X1
# ─────────────────────────────────────────────────────────────────────────────
def pico_y_for_pin(pin_name):
    for i, (_, gpio, sig, _) in enumerate(pins):
        if gpio == pin_name or sig.startswith(pin_name):
            return PICO_Y1 + 60 + 30 + i * ROW_H + 1
    return None

gpio_to_trunk = {
    "GP18": "SCK",
    "GP19": "MOSI",
    "GP20": "DC",
    "GP21": "RST",
    "3V3":  "VCC",
    "GND":  "GND",
    "GP13": "CS1",
    "GP14": "CS2",
    "GP15": "CS3",
}

for gpio, trunk in gpio_to_trunk.items():
    py = pico_y_for_pin(gpio)
    tx = trunk_xs[trunk]
    if py:
        col = next(c for (_, g, _, c) in pins if g == gpio)
        hline(PICO_X2, tx, py, col, width=2)
        d.ellipse([s(tx - 4), s(py - 4), s(tx + 4), s(py + 4)], fill=col)

# ─────────────────────────────────────────────────────────────────────────────
# Display boxes helper
# ─────────────────────────────────────────────────────────────────────────────
def draw_display(n, x1, y1, x2, y2, cs_gpio, cs_colour):
    rect(x1, y1, x2, y2, BOX_DISP, BOX_DISP_BD)
    cy = (y1 + y2) // 2
    text((x1 + x2) // 2, y1 + 14, f"ST7735 Display #{n}",
         sz=13, bold=True, colour=BOX_DISP_BD, anchor="ma")

    # pin labels inside the box
    rows = [
        (f"CS  ←  {cs_gpio}", cs_colour),
        ("SCK  ←  GP18",   C_SCK),
        ("MOSI ←  GP19",   C_MOSI),
        ("DC   ←  GP20",   C_DC),
        ("RST  ←  GP21",   C_RST),
        ("VCC  ←  3V3",    C_PWR),
        ("GND  ←  GND",    C_GND),
    ]
    row_h = (y2 - y1 - 44) // len(rows)
    for j, (lbl, col) in enumerate(rows):
        text(x1 + 12, y1 + 38 + j * row_h, lbl, sz=11, colour=col)

    return cy

D1_CY = draw_display(1, DISP_X1, D1_Y1, DISP_X2, D1_Y2, "GP13", C_CS1)
D2_CY = draw_display(2, DISP_X1, D2_Y1, DISP_X2, D2_Y2, "GP14", C_CS2)
D3_CY = draw_display(3, DISP_X1, D3_Y1, DISP_X2, D3_Y2, "GP15", C_CS3)

# ─────────────────────────────────────────────────────────────────────────────
# Horizontal connections from trunks to display left edges
# ─────────────────────────────────────────────────────────────────────────────
shared_signals = ["SCK", "MOSI", "DC", "RST", "VCC", "GND"]
cs_map = {1: "CS1", 2: "CS2", 3: "CS3"}
disp_ys = {
    1: (D1_Y1, D1_Y2),
    2: (D2_Y1, D2_Y2),
    3: (D3_Y1, D3_Y2),
}
cs_colours = {1: C_CS1, 2: C_CS2, 3: C_CS3}

for n in range(1, 4):
    y1, y2 = disp_ys[n]
    row_h = (y2 - y1 - 44) // 7
    # shared signals rows 1-6 (index 1..6 inside box)
    for j, sig in enumerate(shared_signals):
        row_y = y1 + 38 + (j + 1) * row_h    # +1 because CS is row 0
        tx = trunk_xs[sig]
        col = next(c for (name, c) in signals if name == sig)
        hline(tx, DISP_X1, row_y, col, width=2)
        d.ellipse([s(tx - 4), s(row_y - 4), s(tx + 4), s(row_y + 4)], fill=col)

    # CS line – row 0
    cs_row_y = y1 + 38
    cs_sig = cs_map[n]
    tx = trunk_xs[cs_sig]
    col = cs_colours[n]
    hline(tx, DISP_X1, cs_row_y, col, width=2)
    d.ellipse([s(tx - 4), s(cs_row_y - 4), s(tx + 4), s(cs_row_y + 4)], fill=col)

# ─────────────────────────────────────────────────────────────────────────────
# Motor circuit box
# ─────────────────────────────────────────────────────────────────────────────
rect(MOT_X1, MOT_Y1, MOT_X2, MOT_Y2, BOX_MOTOR, BOX_MOTOR_BD)
text((MOT_X1 + MOT_X2) // 2, MOT_Y1 + 16,
     "Motor circuit", sz=14, bold=True, colour=BOX_MOTOR_BD, anchor="ma")

motor_rows = [
    ("IN  ← GP16 (3.3 V logic)", C_MOTOR),
    ("NPN transistor / N-MOSFET", BOX_MOTOR_BD),
    ("or relay module", BOX_MOTOR_BD),
    ("", TXT),
    ("External supply V+", C_PWR),
    ("switched to motor load", BOX_MOTOR_BD),
    ("", TXT),
    ("Flyback diode 1N4007", (180, 0, 0)),
    ("across inductive load", BOX_MOTOR_BD),
    ("", TXT),
    ("GND → Pico GND", C_GND),
]
for j, (lbl, col) in enumerate(motor_rows):
    text(MOT_X1 + 12, MOT_Y1 + 42 + j * 33, lbl, sz=11, colour=col)

# Motor signal horizontal line from Pico GP16
motor_py = pico_y_for_pin("GP16")
MOT_MID_Y = MOT_Y1 + 58
if motor_py:
    hline(PICO_X2, MOT_X1, motor_py, C_MOTOR, width=2)
    # vertical jog to motor box top
    vline(MOT_X1 + 5, motor_py, MOT_MID_Y, C_MOTOR, width=2)
    hline(MOT_X1 + 5, MOT_X1 + 40, MOT_MID_Y, C_MOTOR, width=2)
    d.polygon(
        [s((MOT_X1 + 40, MOT_MID_Y)), s((MOT_X1 + 34, MOT_MID_Y - 4)),
         s((MOT_X1 + 34, MOT_MID_Y + 4))],
        fill=C_MOTOR,
    )

# ─────────────────────────────────────────────────────────────────────────────
# Note box
# ─────────────────────────────────────────────────────────────────────────────
rect(NOTE_X1, NOTE_Y1, NOTE_X2, NOTE_Y2, BOX_NOTE, BOX_NOTE_BD, radius=5)
text((NOTE_X1 + NOTE_X2) // 2, NOTE_Y1 + 14,
     "Notes", sz=12, bold=True, colour=BOX_NOTE_BD, anchor="ma")
notes = [
    "MISO not required.",
    "MOSI shared on all 3 displays.",
    "Each CS line is individual.",
    "Use separate 3.3 V supply if",
    "3 TFT backlights exceed Pico",
    "3V3 current budget.",
]
for j, n in enumerate(notes):
    text(NOTE_X1 + 10, NOTE_Y1 + 34 + j * 20, n, sz=10, colour=TXT)

# ─────────────────────────────────────────────────────────────────────────────
# Title
# ─────────────────────────────────────────────────────────────────────────────
text(W // 2, 14, "JUMO Tower – Hardware Connection Diagram",
     sz=18, bold=True, colour=(30, 30, 80), anchor="ma")

# ─────────────────────────────────────────────────────────────────────────────
# Legend
# ─────────────────────────────────────────────────────────────────────────────
legend_items = [
    ("SCK",  C_SCK),
    ("MOSI", C_MOSI),
    ("DC",   C_DC),
    ("RST",  C_RST),
    ("VCC",  C_PWR),
    ("GND",  C_GND),
    ("CS1",  C_CS1),
    ("CS2",  C_CS2),
    ("CS3",  C_CS3),
    ("Motor GP16", C_MOTOR),
]
LX = 30
LY = H - 45
text(LX, LY - 2, "Legend:", sz=11, bold=True)
for i, (name, col) in enumerate(legend_items):
    ox = LX + 70 + i * 140
    d.rectangle([s(ox), s(LY + 2), s(ox + 30), s(LY + 12)], fill=col)
    text(ox + 35, LY + 6, name, sz=10, colour=col)

# ─────────────────────────────────────────────────────────────────────────────
# Save
# ─────────────────────────────────────────────────────────────────────────────
out = img.resize((W, H), Image.LANCZOS)
out.save("schematic.drawio.png", dpi=(150, 150))
print("schematic.drawio.png written")
