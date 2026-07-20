#!/usr/bin/env python3
"""Generate schematic.drawio.png – clear connection diagram for the JUMO Tower (v2)."""

from PIL import Image, ImageDraw, ImageFont

# ── Fonts ────────────────────────────────────────────────────────────────────
FONT_REGULAR = "/usr/share/fonts/adwaita-sans-fonts/AdwaitaSans-Regular.ttf"
FONT_BOLD    = "/usr/share/fonts/google-droid-sans-fonts/DroidSans-Bold.ttf"

def font(size, bold=False):
    return ImageFont.truetype(FONT_BOLD if bold else FONT_REGULAR, size)

# ── Colours ──────────────────────────────────────────────────────────────────
BG           = (250, 250, 252)
BOX_PICO     = (215, 228, 248)
BOX_PICO_BD  = (50,  80,  160)
BOX_DISP     = (212, 242, 222)
BOX_DISP_BD  = (35,  125, 65)
BOX_MOTOR    = (255, 238, 205)
BOX_MOTOR_BD = (175, 100, 15)
BOX_NOTE     = (252, 248, 210)
BOX_NOTE_BD  = (150, 135, 30)

C_SCK   = (20,  80,  200)
C_MOSI  = (0,   150, 90)
C_DC    = (130, 30,  140)
C_RST   = (175, 50,  50)
C_PWR   = (200, 20,  20)
C_GND   = (55,  55,  55)
C_CS1   = (0,   155, 200)
C_CS2   = (0,   110, 170)
C_CS3   = (0,   65,  135)
C_MOTOR = (185, 85,  0)
TXT     = (18,  18,  18)

# ── Canvas ───────────────────────────────────────────────────────────────────
W, H = 1700, 1000
SCALE = 2
img = Image.new("RGB", (W * SCALE, H * SCALE), BG)
d   = ImageDraw.Draw(img)

def s(v):
    if isinstance(v, (list, tuple)):
        return type(v)(s(i) for i in v)
    return int(v * SCALE)

def rect(x1, y1, x2, y2, fill, bd, r=8):
    d.rounded_rectangle([s(x1), s(y1), s(x2), s(y2)],
                        radius=s(r), fill=fill, outline=bd, width=s(2))

def txt(x, y, t, sz=12, bold=False, c=TXT, anc="lt"):
    d.text((s(x), s(y)), t, fill=c, font=font(s(sz), bold), anchor=anc)

def hl(x1, x2, y, c, w=2):
    d.line([(s(x1), s(y)), (s(x2), s(y))], fill=c, width=s(w))

def vl(x, y1, y2, c, w=2):
    d.line([(s(x), s(y1)), (s(x), s(y2))], fill=c, width=s(w))

def dot(x, y, c, r=5):
    d.ellipse([s(x-r), s(y-r), s(x+r), s(y+r)], fill=c)

def arrowhead_right(x, y, c):
    d.polygon([s((x, y)), s((x-8, y-5)), s((x-8, y+5))], fill=c)

def arrowhead_down(x, y, c):
    d.polygon([s((x, y)), s((x-5, y-8)), s((x+5, y-8))], fill=c)

# ─────────────────────────────────────────────────────────────────────────────
# Geometry
# ─────────────────────────────────────────────────────────────────────────────
TITLE_H  = 42
LEG_H    = 44   # legend strip at bottom

# Pico box
P_X1, P_X2 = 30, 300
P_Y1        = TITLE_H + 8
ROW_H       = 72
NHDR        = 10   # number of pin rows
P_HDR_H     = 66   # space above first pin row (title + column header)
P_USB_H     = 68   # USB sub-box at bottom
P_Y2        = P_Y1 + P_HDR_H + NHDR * ROW_H + P_USB_H

PIN_Y0 = P_Y1 + P_HDR_H  # y of top of first pin row

def pin_cy(i):
    """Centre y of pin row i."""
    return PIN_Y0 + i * ROW_H + ROW_H // 2

# Displays
D_X1, D_X2 = 650, 920
D_HDR       = 38
D_NROWS     = 7          # CS SCK MOSI DC RST VCC GND
D_ROW_H     = 30
D_H         = D_HDR + D_NROWS * D_ROW_H   # = 248
D_GAP       = 34

# Anchor display 1 so row 0 (CS) aligns with pin_cy(0) (GP13)
D1_Y1 = pin_cy(0) - D_HDR - D_ROW_H // 2
D2_Y1 = D1_Y1 + D_H + D_GAP
D3_Y1 = D2_Y1 + D_H + D_GAP
DY1 = [D1_Y1, D2_Y1, D3_Y1]
DY2 = [y + D_H for y in DY1]

def d_row_cy(n, r):
    """Centre y of row r (0=CS) in display n."""
    return DY1[n] + D_HDR + r * D_ROW_H + D_ROW_H // 2

# Motor box (right side, top-aligned)
M_X1, M_X2 = 1000, 1260
M_Y1        = TITLE_H + 8
M_Y2        = M_Y1 + 400

# Notes box
N_X1, N_X2 = 1000, 1260
N_Y1        = M_Y2 + 20
N_Y2        = N_Y1 + 180

# Shared bus trunk x positions  (SCK MOSI DC RST VCC GND)
SHARED = [("SCK",C_SCK),("MOSI",C_MOSI),("DC",C_DC),
          ("RST",C_RST),("VCC",C_PWR),("GND",C_GND)]
T_X0   = 340  # left edge of first trunk
T_SEP  = 29
TX     = {name: T_X0 + i * T_SEP for i, (name, _) in enumerate(SHARED)}

# ─────────────────────────────────────────────────────────────────────────────
# Title
# ─────────────────────────────────────────────────────────────────────────────
txt(W//2, 10, "JUMO Tower – Hardware Connections",
    sz=17, bold=True, c=(25,25,75), anc="mt")

# ─────────────────────────────────────────────────────────────────────────────
# Pico box
# ─────────────────────────────────────────────────────────────────────────────
rect(P_X1, P_Y1, P_X2, P_Y2, BOX_PICO, BOX_PICO_BD)
txt((P_X1+P_X2)//2, P_Y1+10, "Raspberry Pi Pico",
    sz=14, bold=True, c=BOX_PICO_BD, anc="mt")
txt((P_X1+P_X2)//2, P_Y1+28, "(RP2040)",
    sz=11, c=BOX_PICO_BD, anc="mt")

# Column headers
PX_NR  = P_X1 + 12
PX_GPIO= P_X1 + 46
PX_SIG = P_X1 + 97
y_hdr  = P_Y1 + P_HDR_H - 14
txt(PX_NR,  y_hdr, "Pin",    sz=10, bold=True)
txt(PX_GPIO,y_hdr, "GPIO",   sz=10, bold=True)
txt(PX_SIG, y_hdr, "Signal", sz=10, bold=True)

pins = [
    # (nr, gpio, label, colour)
    ("17","GP13","CS Display #1", C_CS1),
    ("19","GP14","CS Display #2", C_CS2),
    ("20","GP15","CS Display #3", C_CS3),
    ("24","GP18","SPI SCK",       C_SCK),
    ("25","GP19","SPI MOSI/SDA",  C_MOSI),
    ("26","GP20","DC / A0",       C_DC),
    ("27","GP21","RST / RES",     C_RST),
    ("36","3V3", "Display VCC",   C_PWR),
    ("38","GND", "Common GND",    C_GND),
    ("21","GP16","Motor output",  C_MOTOR),
]

for i,(nr,gpio,sig,c) in enumerate(pins):
    cy = pin_cy(i)
    txt(PX_NR,  cy-6, nr,   sz=11, c=TXT)
    txt(PX_GPIO,cy-6, gpio, sz=12, bold=True, c=c)
    txt(PX_SIG, cy-6, sig,  sz=11, c=c)
    dot(P_X2, cy, c, r=4)

# USB sub-box
ub_y1 = P_Y2 - P_USB_H + 6
rect(P_X1+12, ub_y1, P_X2-12, P_Y2-8, (232,232,250), BOX_PICO_BD, r=5)
txt((P_X1+P_X2)//2, ub_y1+10, "USB CDC-ACM",
    sz=11, bold=True, c=BOX_PICO_BD, anc="mt")
txt((P_X1+P_X2)//2, ub_y1+26, "Modbus  /dev/ttyACM0",
    sz=9, c=BOX_PICO_BD, anc="mt")
txt((P_X1+P_X2)//2, ub_y1+42, "Debug   /dev/ttyACM1",
    sz=9, c=BOX_PICO_BD, anc="mt")

# ─────────────────────────────────────────────────────────────────────────────
# Shared bus trunks (vertical lines + top labels)
# ─────────────────────────────────────────────────────────────────────────────
T_TOP = TITLE_H + 24
T_BOT = DY2[2] + 10

for name, c in SHARED:
    tx = TX[name]
    vl(tx, T_TOP, T_BOT, c, w=2)
    txt(tx, T_TOP-2, name, sz=8, c=c, anc="mb")

# Pico → trunk: horizontal from pin exit dot to trunk + junction dot
shared_gpio = {"GP18":"SCK","GP19":"MOSI","GP20":"DC",
               "GP21":"RST","3V3":"VCC","GND":"GND"}
for i,(nr,gpio,sig,c) in enumerate(pins):
    if gpio in shared_gpio:
        cy = pin_cy(i)
        tx = TX[shared_gpio[gpio]]
        hl(P_X2, tx, cy, c, w=2)
        dot(tx, cy, c, r=4)

# ─────────────────────────────────────────────────────────────────────────────
# CS individual L-routes (right → down → right)
# Each CS routes in its own vertical lane, well clear of display left edge
# ─────────────────────────────────────────────────────────────────────────────
CS_LANES = {0: 510, 1: 536, 2: 562}   # x of vertical segment per CS
CS_INFO  = [(0,0,C_CS1,"GP13"),(1,1,C_CS2,"GP14"),(2,2,C_CS3,"GP15")]

for pin_i, disp_n, c, gpio in CS_INFO:
    py  = pin_cy(pin_i)
    ty  = d_row_cy(disp_n, 0)
    lx  = CS_LANES[pin_i]
    # Pico exit → lane
    hl(P_X2, lx, py, c, w=2)
    # vertical drop (or rise) in lane
    vl(lx, py, ty, c, w=2)
    # lane → display left edge
    hl(lx, D_X1, ty, c, w=2)
    arrowhead_right(D_X1, ty, c)

# ─────────────────────────────────────────────────────────────────────────────
# Motor routing: right → up → right → down into motor box
# Route goes ABOVE all displays and bus (uses clear space at top)
# ─────────────────────────────────────────────────────────────────────────────
MOT_PIN_I = 9
mot_py    = pin_cy(MOT_PIN_I)

# vertical routing corridor x (between Pico and bus, no conflicts)
MOT_UP_X  = P_X2 + 18   # just right of Pico right edge
MOT_TOP_Y = TITLE_H + 6  # between title text and trunk labels

mot_entry_x = (M_X1 + M_X2) // 2  # enter motor box from top-centre

hl(P_X2,       MOT_UP_X,    mot_py,     C_MOTOR, w=2)   # short stub right
vl(MOT_UP_X,   mot_py,      MOT_TOP_Y,  C_MOTOR, w=2)   # up
hl(MOT_UP_X,   mot_entry_x, MOT_TOP_Y,  C_MOTOR, w=2)   # right across top
vl(mot_entry_x, MOT_TOP_Y,  M_Y1,       C_MOTOR, w=2)   # down into box
arrowhead_down(mot_entry_x, M_Y1, C_MOTOR)

# small label on top horizontal segment
txt(MOT_UP_X + 8, MOT_TOP_Y - 2, "GP16  Motor signal",
    sz=10, c=C_MOTOR, anc="lb")

# ─────────────────────────────────────────────────────────────────────────────
# Display boxes + shared-bus horizontal branches
# ─────────────────────────────────────────────────────────────────────────────
DISP_SIGNALS = [
    (None,   None),   # row 0 = CS (handled above)
    ("SCK",  C_SCK),
    ("MOSI", C_MOSI),
    ("DC",   C_DC),
    ("RST",  C_RST),
    ("VCC",  C_PWR),
    ("GND",  C_GND),
]
CS_GPIO_LBL = ["GP13","GP14","GP15"]
CS_COLOURS  = [C_CS1, C_CS2, C_CS3]

for n in range(3):
    x1, x2 = D_X1, D_X2
    y1, y2 = DY1[n], DY2[n]
    rect(x1, y1, x2, y2, BOX_DISP, BOX_DISP_BD)
    txt((x1+x2)//2, y1+10, f"ST7735  Display #{n+1}",
        sz=13, bold=True, c=BOX_DISP_BD, anc="mt")

    for r,(sig,c) in enumerate(DISP_SIGNALS):
        ry = d_row_cy(n, r)
        if r == 0:
            lbl = f"CS   ←  {CS_GPIO_LBL[n]}"
            c   = CS_COLOURS[n]
        else:
            gpio_map = {"SCK":"GP18","MOSI":"GP19","DC":"GP20",
                        "RST":"GP21","VCC":"3V3","GND":"GND"}
            lbl = f"{sig:<4} ←  {gpio_map[sig]}"
            # horizontal branch from trunk to display
            tx = TX[sig]
            hl(tx, x1, ry, c, w=2)
            dot(tx, ry, c, r=4)
            arrowhead_right(x1, ry, c)

        txt(x1+10, ry-7, lbl, sz=11, c=c)

# ─────────────────────────────────────────────────────────────────────────────
# Motor box
# ─────────────────────────────────────────────────────────────────────────────
rect(M_X1, M_Y1, M_X2, M_Y2, BOX_MOTOR, BOX_MOTOR_BD)
txt((M_X1+M_X2)//2, M_Y1+10, "Motor Driver Circuit",
    sz=14, bold=True, c=BOX_MOTOR_BD, anc="mt")

motor_lines = [
    ("IN  ← GP16  (3.3 V logic)",  C_MOTOR),
    ("",                            TXT),
    ("NPN / N-MOSFET or relay",     BOX_MOTOR_BD),
    ("  module with driver",        BOX_MOTOR_BD),
    ("",                            TXT),
    ("V+  external supply",         C_PWR),
    ("  switched to motor",         BOX_MOTOR_BD),
    ("",                            TXT),
    ("Flyback diode 1N4007",        (190,20,20)),
    ("  across inductive load!",    (190,20,20)),
    ("",                            TXT),
    ("GND → common Pico GND",       C_GND),
]
for j,(lbl,c) in enumerate(motor_lines):
    txt(M_X1+14, M_Y1+40+j*27, lbl, sz=11, c=c)

# ─────────────────────────────────────────────────────────────────────────────
# Notes box
# ─────────────────────────────────────────────────────────────────────────────
rect(N_X1, N_Y1, N_X2, N_Y2, BOX_NOTE, BOX_NOTE_BD, r=6)
txt((N_X1+N_X2)//2, N_Y1+10, "Notes",
    sz=12, bold=True, c=BOX_NOTE_BD, anc="mt")
notes = [
    "MISO not required.",
    "SCK, MOSI, DC, RST, VCC, GND",
    "  are shared on all 3 displays.",
    "Each display has its own CS line.",
    "Line crossing without dot = no connection.",
    "Use separate 3.3V supply if 3 TFTs",
    "  exceed Pico 3V3 current budget.",
]
for j,n in enumerate(notes):
    txt(N_X1+10, N_Y1+32+j*20, n, sz=10, c=TXT)

# ─────────────────────────────────────────────────────────────────────────────
# Legend strip
# ─────────────────────────────────────────────────────────────────────────────
LY = H - LEG_H + 8
txt(14, LY, "Legend:", sz=11, bold=True, c=TXT)
items = [("SCK",C_SCK),("MOSI",C_MOSI),("DC",C_DC),("RST",C_RST),
         ("VCC",C_PWR),("GND",C_GND),("CS1",C_CS1),("CS2",C_CS2),
         ("CS3",C_CS3),("GP16 Motor",C_MOTOR)]
for i,(lbl,c) in enumerate(items):
    ox = 80 + i * 155
    d.rectangle([s(ox), s(LY+4), s(ox+28), s(LY+15)], fill=c)
    txt(ox+33, LY+6, lbl, sz=10, c=c)

# ─────────────────────────────────────────────────────────────────────────────
# Save
# ─────────────────────────────────────────────────────────────────────────────
out = img.resize((W, H), Image.LANCZOS)
out.save("schematic.png", dpi=(150, 150))
print(f"schematic.png written  {W}x{H}")
