#!/usr/bin/env python3
# Generates docs/pictures/16_wiring.svg: cartoon wiring in pinout.xyz style, pin accurate
import sys

W, H = 1290, 495
S = 8.0                                  # px per mm
PITCH = 2.54 * S
PX, PY = 400, 60                         # Pico top-left
PW, PH = 21 * S, 51 * S
PILL_W, PILL_H, PILL_GAP = 84, 16, 8

LEFT = ["GP0", "GP1", "GND", "GP2", "GP3", "GP4", "GP5", "GND", "GP6", "GP7",
        "GP8", "GP9", "GND", "GP10", "GP11", "GP12", "GP13", "GND", "GP14", "GP15"]
RIGHT = ["VBUS", "VSYS", "GND", "3V3_EN", "3V3", "ADC_VREF", "GP28", "AGND", "GP27", "GP26",
         "RUN", "GP22", "GND", "GP21", "GP20", "GP19", "GP18", "GND", "GP17", "GP16"]
USED_LEFT = {15, 16, 17, 19}             # GP12, GP13, GND (pin 18), GP15
USED_RIGHT = {1, 2, 15, 16, 17, 18, 19}  # VSYS, GND (pin 38), GP19, GP18, GND (pin 23), GP17, GP16

C_5V, C_GND = "#d7263d", "#222222"
C_V1A, C_V1B, C_V2A, C_V2B = "#1f77b4", "#5fb3e8", "#e8890c", "#f2c14e"
C_SLP, C_RED, C_GRN = "#8e44ad", "#d7263d", "#2e9e4f"

out = []
a = out.append


def py(i):
    return PY + (1.37 + i * 2.54) * S


def kind(n):
    if n in ("GND", "AGND"):
        return "#1a1a1a", "#ffffff"
    if n in ("VBUS", "VSYS"):
        return "#d7263d", "#ffffff"
    if n in ("3V3", "3V3_EN"):
        return "#e36414", "#ffffff"
    if n.startswith("GP"):
        return "#3a9a5b", "#ffffff"
    if n.startswith("OUT"):
        return "#b5651d", "#ffffff"
    if n in ("VCC",):
        return "#d7263d", "#ffffff"
    return "#6c757d", "#ffffff"


def text(x, y, s, size=12, color="#222", anchor="start", weight="normal"):
    a('<text x="%.1f" y="%.1f" font-size="%s" fill="%s" text-anchor="%s" font-weight="%s">%s</text>'
      % (x, y, size, color, anchor, weight, s))


def pill(edge_x, y, name, num, side, used):
    """Pin label sticking out of a board edge; returns the outer end x."""
    bg, fg = kind(name)
    x = edge_x - PILL_GAP - PILL_W if side < 0 else edge_x + PILL_GAP
    op = "1" if used else "0.35"
    a('<g opacity="%s">' % op)
    a('<line x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f" stroke="%s" stroke-width="2"/>'
      % (edge_x, y, x + (PILL_W if side < 0 else 0), y, bg))
    a('<rect x="%.1f" y="%.1f" width="%d" height="%d" rx="8" fill="%s"/>' % (x, y - PILL_H / 2, PILL_W, PILL_H, bg))
    if num is not None:
        nx = x + PILL_W - 9 if side < 0 else x + 9
        a('<circle cx="%.1f" cy="%.1f" r="7" fill="#ffffff" opacity="0.9"/>' % (nx, y))
        text(nx, y + 3.5, str(num), 8.5, bg, "middle", "bold")
        tx = x + (PILL_W - 20) / 2 if side < 0 else x + 20 + (PILL_W - 20) / 2
    else:
        tx = x + PILL_W / 2
    text(tx, y + 4.5, name, 11, fg, "middle", "bold" if used else "normal")
    a('</g>')
    return x if side < 0 else x + PILL_W


def wire(pts, color, w=3.5, over=False):
    d = " ".join("%.1f,%.1f" % p for p in pts)
    if over:                             # halo so it reads as passing over other wires
        a('<polyline fill="none" stroke="#fbfaf6" stroke-width="%.1f" stroke-linejoin="round" points="%s"/>' % (w + 8, d))
    a('<polyline fill="none" stroke="#00000033" stroke-width="%.1f" stroke-linejoin="round" stroke-linecap="round" points="%s"/>' % (w + 2, d))
    a('<polyline fill="none" stroke="%s" stroke-width="%.1f" stroke-linejoin="round" stroke-linecap="round" points="%s"/>' % (color, w, d))


def dot(x, y, color):
    a('<circle cx="%.1f" cy="%.1f" r="5" fill="%s" stroke="#ffffff" stroke-width="1.5"/>' % (x, y, color))


X0 = 90                                  # crop: same margin left of the GP15 wire as on the right
a('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="%d 0 %d %d" '
  'font-family="Helvetica, Arial, sans-serif">' % (W - X0, H, X0, W - X0, H))
a('<rect x="%d" width="%d" height="%d" rx="14" fill="#fbfaf6"/>' % (X0, W - X0, H))
# easter egg: faint blurred PicoHive in the empty middle
a('<defs><filter id="blur" x="-20%" y="-50%" width="140%" height="200%"><feGaussianBlur stdDeviation="1.4"/></filter></defs>')
a('<g filter="url(#blur)"><path transform="translate(850 252) scale(1.1)" d="M16 2l12 7v14l-12 7-12-7V9z" fill="#F2A516"/>'
  '<path transform="translate(850 252) scale(1.1)" d="M16 9l6 3.5v7L16 23l-6-3.5v-7z" fill="#2A1F0E"/>'
  '<text x="892" y="279" font-size="26" font-weight="bold" fill="#b8860b">PicoHive</text></g>')

# ---------------------------------------------------------------- Pico W (USB up, face up)
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="10" fill="#2f8f4e" stroke="#1d5e32" stroke-width="3"/>' % (PX, PY, PW, PH))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="4" fill="#d5d8dc" stroke="#7f8c8d" stroke-width="2"/>'
  % (PX + PW / 2 - 4 * S, PY - 1.6 * S, 8 * S, 5.6 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="2" fill="#95a5a6"/>' % (PX + PW / 2 - 3 * S, PY - 0.4 * S, 6 * S, 1.4 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="4" fill="#f4f4f4" stroke="#b0b0b0" stroke-width="1.5"/>' % (PX + 4.6 * S, PY + 8.5 * S, 4 * S, 4.6 * S))
a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#ffffff" stroke="#c8c8c8"/>' % (PX + 6.6 * S, PY + 10.8 * S, 1.2 * S))
text(PX + 6.6 * S, PY + 15.4 * S, "BOOTSEL", 7.5, "#e8f5e9", "middle", "bold")
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="6" fill="#2b2b2b" stroke="#111" stroke-width="2"/>' % (PX + 6.5 * S, PY + 18 * S, 8 * S, 8 * S))
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="6" fill="#dfe4e8" stroke="#9aa4ab" stroke-width="2"/>' % (PX + 5.5 * S, PY + 31 * S, 10 * S, 9.5 * S))
text(PX + 10.5 * S, PY + 36.4 * S, "WiFi", 11, "#6b7880", "middle", "bold")
a('<path d="M%.1f %.1f q%.1f -%.1f %.1f 0 t%.1f 0 t%.1f 0 t%.1f 0" fill="none" stroke="#e9c46a" stroke-width="3" stroke-linecap="round"/>'
  % (PX + 4.5 * S, PY + 46.5 * S, 1.5 * S, 2.5 * S, 3 * S, 3 * S, 3 * S, 3 * S))
# PicoHive hex as a silkscreen logo, where the LED would be
a('<g transform="translate(%.1f %.1f) scale(0.75)"><path d="M16 2l12 7v14l-12 7-12-7V9z" fill="#F2A516"/>'
  '<path d="M16 9l6 3.5v7L16 23l-6-3.5v-7z" fill="#2A1F0E"/></g>' % (PX + 14 * S - 12, PY + 6.5 * S - 12))
text(PX + PW / 2, PY + 29.3 * S, "Pico W / Pico 2 W", 10, "#ffffff", "middle", "bold")

def pad(x, y):
    """Plated through hole."""
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#e9c46a" stroke="#a07c1c" stroke-width="1.5"/>' % (x, y, 0.9 * S))
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#1b1b1b"/>' % (x, y, 0.45 * S))


# mounting holes
for mx, my in ((4.8, 2), (16.2, 2), (4.8, 49), (16.2, 49)):
    a('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#fbfaf6" stroke="#c9a64a" stroke-width="2.5"/>' % (PX + mx * S, PY + my * S, 1.05 * S))

PL, PR = PX + 1.61 * S, PX + PW - 1.61 * S
LEND, REND = {}, {}
for i, n in enumerate(LEFT):
    pad(PL, py(i))
    LEND[i] = pill(PX, py(i), n, i + 1, -1, i in USED_LEFT)
for i, n in enumerate(RIGHT):
    pad(PR, py(i))
    REND[i] = pill(PX + PW, py(i), n, 40 - i, 1, i in USED_RIGHT)

# ---------------------------------------------------------------- DRV8833 module
DLEFT = ["IN4", "IN3", "GND", "VCC", "IN2", "IN1"]
DRIGHT = ["EEP", "OUT1", "OUT2", "OUT3", "OUT4", "ULT"]
DX, DW = 860, 130
D0 = py(14)                              # one row up, so the bottom lines up with the Pico


def dy(i):
    return D0 + i * PITCH


DY, DH = D0 - 11, 5 * PITCH + 22
a('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="10" fill="#26262b" stroke="#000" stroke-width="3"/>' % (DX, DY, DW, DH))
a('<rect x="%.1f" y="%.1f" width="40" height="52" rx="5" fill="#3a3a40" stroke="#111" stroke-width="2"/>' % (DX + DW / 2 - 20, DY + DH / 2 - 20))
for k in range(6):
    for sx in (DX + DW / 2 - 26, DX + DW / 2 + 20):
        a('<rect x="%.1f" y="%.1f" width="6" height="3" fill="#c0c0c0"/>' % (sx, DY + DH / 2 - 16 + k * 8))
text(DX + DW / 2, DY + 26, "DRV8833", 12, "#f0f0f0", "middle", "bold")
DLEND, DREND = {}, {}
for i, n in enumerate(DLEFT):
    pad(DX + 12, dy(i))
    DLEND[i] = pill(DX, dy(i), n, None, -1, True)
for i, n in enumerate(DRIGHT):
    pad(DX + DW - 12, dy(i))
    DREND[i] = pill(DX + DW, dy(i), n, None, 1, n != "ULT")

# ---------------------------------------------------------------- Pico -> DRV8833
wire([(REND[15], py(15)), (680, py(15)), (680, dy(0)), (DLEND[0], dy(0))], C_V2B)              # GP19 -> IN4
wire([(REND[16], py(16)), (695, py(16)), (695, dy(1)), (DLEND[1], dy(1))], C_V2A)              # GP18 -> IN3
wire([(REND[17], py(17)), (710, py(17)), (710, dy(2)), (DLEND[2], dy(2))], C_GND)              # GND  -> GND
wire([(REND[18], py(18)), (DLEND[4], dy(4))], C_V1B)                                         # GP17 -> IN2
wire([(REND[19], py(19)), (DLEND[5], dy(5))], C_V1A)                                         # GP16 -> IN1

# ---------------------------------------------------------------- Power
SX, SY = 1120, PY + 2
y5, yg = py(1), py(2)
wire([(REND[1], y5), (SX, y5)], C_5V)
wire([(REND[2], yg), (SX, yg)], C_GND)
wire([(752, y5), (752, dy(3)), (DLEND[3], dy(3))], C_5V, over=True)                           # 5V -> VCC
dot(752, y5, C_5V)
# bulk capacitor across VSYS/GND, + lead passes over GND
CX = 860
wire([(CX - 8, y5), (CX - 8, yg + 24)], "#9aa3ab", 2.5, over=True)
wire([(CX + 8, yg), (CX + 8, yg + 24)], "#9aa3ab", 2.5)
a('<rect x="%.1f" y="%.1f" width="30" height="44" rx="6" fill="#2d5fa8" stroke="#1b3d6e" stroke-width="2"/>' % (CX - 15, yg + 22))
a('<rect x="%.1f" y="%.1f" width="9" height="44" fill="#a9c4ea"/>' % (CX + 4, yg + 22))
a('<ellipse cx="%.1f" cy="%.1f" rx="15" ry="4" fill="#5b86c8" stroke="#1b3d6e" stroke-width="1.5"/>' % (CX, yg + 23))
text(CX - 8, yg + 52, "+", 11, "#ffffff", "middle", "bold")
text(CX + 22, yg + 44, "3500uF", 11, "#1b3d6e", "start", "bold")
dot(CX - 8, y5, C_5V)
dot(CX + 8, yg, C_GND)
# polyfuses on both supply lines
for yy in (y5, yg):
    a('<rect x="%.1f" y="%.1f" width="34" height="16" rx="8" fill="#f2c14e" stroke="#b8860b" stroke-width="2"/>' % (980, yy - 8))
text(997, y5 - 14, "polyfuses", 10, "#8a6a10", "middle")
# supply brick
a('<rect x="%d" y="%d" width="140" height="70" rx="12" fill="#ffffff" stroke="#555" stroke-width="3"/>' % (SX, SY))
a('<rect x="%d" y="%d" width="22" height="30" rx="3" fill="#cfd4d8" stroke="#555" stroke-width="2"/>' % (SX + 104, SY + 20))
text(SX + 52, SY + 30, "5V USB", 13, "#222", "middle", "bold")
text(SX + 52, SY + 47, "supply", 12, "#555", "middle")
text(SX + 8, y5 + 4, "+", 13, C_5V, "start", "bold")
text(SX + 8, yg + 4, "−", 13, C_GND, "start", "bold")

# GP15 -> EEP (sleep), over the top
LANE = PY - 30
wire([(LEND[19], py(19)), (120, py(19)), (120, LANE), (1100, LANE), (1100, dy(0)), (DREND[0], dy(0))], C_SLP, over=True)

# ---------------------------------------------------------------- Valves (drawn as motors)
def motor(x, ya, yb, title, sub):
    m = (ya + yb) / 2
    for yy in (ya, yb):
        a('<rect x="%.1f" y="%.1f" width="12" height="6" rx="1" fill="#c0c0c0" stroke="#777"/>' % (x, yy - 3))
    a('<rect x="%.1f" y="%.1f" width="80" height="56" rx="10" fill="#9aa3ab" stroke="#4d565e" stroke-width="2.5"/>' % (x + 10, m - 28))
    a('<rect x="%.1f" y="%.1f" width="14" height="56" rx="5" fill="#6f7a84" stroke="#4d565e" stroke-width="2.5"/>' % (x + 10, m - 28))
    a('<rect x="%.1f" y="%.1f" width="12" height="36" rx="4" fill="#6f7a84" stroke="#4d565e" stroke-width="2.5"/>' % (x + 88, m - 18))
    a('<rect x="%.1f" y="%.1f" width="18" height="6" rx="2" fill="#d0d4d8" stroke="#4d565e" stroke-width="1.5"/>' % (x + 100, m - 3))
    a('<circle cx="%.1f" cy="%.1f" r="15" fill="#ffffff" stroke="#4d565e" stroke-width="2"/>' % (x + 56, m))
    text(x + 56, m + 6, "M", 17, "#4d565e", "middle", "bold")
    text(x + 60, m + 44, title, 12, "#222", "middle", "bold")
    text(x + 60, m + 57, sub, 10, "#666", "middle")


VX = 1135
M1 = dy(1) - 115                         # valve 1 sits above the driver outputs
wire([(DREND[1], dy(1)), (1110, dy(1)), (1110, M1 - 10), (VX, M1 - 10)], C_V1A)
wire([(DREND[2], dy(2)), (1122, dy(2)), (1122, M1 + 10), (VX, M1 + 10)], C_V1B)
motor(VX, M1 - 10, M1 + 10, "Valve 1 (left)", "5V latching")
wire([(DREND[3], dy(3)), (VX, dy(3))], C_V2A)
wire([(DREND[4], dy(4)), (VX, dy(4))], C_V2B)
motor(VX, dy(3), dy(4), "Valve 2 (right)", "5V latching")

# ---------------------------------------------------------------- Buttons (other leg to pin 18 GND)
def button(cx, cy, cap, rim, label, sub):
    a('<circle cx="%.1f" cy="%.1f" r="22" fill="%s" stroke="#00000055" stroke-width="2"/>' % (cx, cy, rim))
    a('<circle cx="%.1f" cy="%.1f" r="15" fill="%s"/>' % (cx, cy, cap))
    a('<ellipse cx="%.1f" cy="%.1f" rx="7" ry="4" fill="#ffffff" opacity="0.45"/>' % (cx - 4, cy - 6))
    text(cx + 28, cy - 8, label, 12, "#222", "start", "bold")
    text(cx, cy + 38, sub, 10, "#666", "middle")


BX = 200
yG, yGND = py(16) - 40, py(17)
yR = yG - (yGND - yG)                    # same gap red-green as green-GND
wire([(LEND[15], py(15)), (275, py(15)), (275, yR), (BX + 22, yR)], C_RED)                # GP12 -> red
wire([(LEND[16], py(16)), (255, py(16)), (255, yG), (BX + 22, yG)], C_GRN)                 # GP13 -> green
wire([(BX - 22, yR), (140, yR), (140, yGND), (LEND[17], yGND)], C_GND)
wire([(BX - 22, yG), (140, yG)], C_GND)
dot(140, yG, C_GND)
button(BX, yR, "#e63946", "#f1f1f1", "Red", "")
button(BX, yG, "#3ecf6a", "#f1f1f1", "Green", "")

a('</svg>')
open(sys.argv[1], "w").write("\n".join(out))
