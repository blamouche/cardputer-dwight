#!/usr/bin/env python3
"""Generate faithful 240x135 mockups of every Cardputer app screen for the README.

These are not device captures: they reproduce the firmware's own draw routines
(glcdfont 5x7 text, RGB565 color quantization, the Dwight ASCII portrait and each
app's layout) so the README visuals match what the device renders ~1:1.

Run:  tools/gen_screenshots.py   (needs Pillow)
Out:  docs/screenshots/*.png
"""
import os
import re
from PIL import Image, ImageDraw

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONT_H = os.path.join(ROOT, ".pio/libdeps/cardputer-adv/M5GFX/src/lgfx/Fonts/glcdfont.h")
ASCII_H = os.path.join(ROOT, "src/core/dwight_ascii.h")
OUT_DIR = os.path.join(ROOT, "docs/screenshots")

DISPLAY_W, DISPLAY_H = 240, 135
SCALE = 4

# --- RGB565 helpers (mirror the device's color path) -----------------------

def to565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

def from565(v):
    r5, g6, b5 = (v >> 11) & 0x1F, (v >> 5) & 0x3F, v & 0x1F
    return ((r5 << 3) | (r5 >> 2), (g6 << 2) | (g6 >> 4), (b5 << 3) | (b5 >> 2))

def lerp565(a, b, t):
    ar, ag, ab = (a >> 11) & 0x1F, (a >> 5) & 0x3F, a & 0x1F
    br, bg, bb = (b >> 11) & 0x1F, (b >> 5) & 0x3F, b & 0x1F
    r = ar + (br - ar) * t // 255
    g = ag + (bg - ag) * t // 255
    bl = ab + (bb - ab) * t // 255
    return (r << 11) | (g << 5) | bl

def rgb(v):
    return from565(v)

# --- Theme (default "green" preset, quantized to 565 like the device) -------

def c(r, g, b):
    return to565(r, g, b)

THEME = {
    "background": c(0x00, 0x00, 0x00),
    "foreground": c(0x33, 0xCC, 0x44),
    "accent":     c(0x40, 0xD8, 0x5C),
    "muted":      c(0x0E, 0x55, 0x24),
    "success":    c(0x33, 0xCC, 0x44),
    "warning":    c(0xE0, 0xB0, 0x00),
    "error":      c(0xCC, 0x33, 0x33),
}
RED_EYE = 0xF800

# --- Font ------------------------------------------------------------------

def load_font():
    txt = open(FONT_H).read()
    body = txt[txt.index("{") + 1: txt.rindex("}")]
    vals = [int(x, 16) for x in re.findall(r"0x[0-9A-Fa-f]{2}", body)]
    # 256 glyphs x 5 columns
    return [vals[i * 5:i * 5 + 5] for i in range(256)]

FONT = load_font()

# --- Screen: a GFX-like canvas at native resolution ------------------------

class Screen:
    def __init__(self):
        self.img = Image.new("RGB", (DISPLAY_W, DISPLAY_H), rgb(THEME["background"]))
        self.d = ImageDraw.Draw(self.img)
        self.tsize = 1
        self.tcolor = THEME["foreground"]
        self.cx = 0
        self.cy = 0

    # primitives -----------------------------------------------------------
    def fill_screen(self, col):
        self.d.rectangle([0, 0, DISPLAY_W, DISPLAY_H], fill=rgb(col))

    def fill_rect(self, x, y, w, h, col):
        if w <= 0 or h <= 0:
            return
        self.d.rectangle([x, y, x + w - 1, y + h - 1], fill=rgb(col))

    def draw_rect(self, x, y, w, h, col):
        self.d.rectangle([x, y, x + w - 1, y + h - 1], outline=rgb(col))

    def fill_round_rect(self, x, y, w, h, r, col):
        self.d.rounded_rectangle([x, y, x + w - 1, y + h - 1], radius=r, fill=rgb(col))

    def draw_round_rect(self, x, y, w, h, r, col):
        self.d.rounded_rectangle([x, y, x + w - 1, y + h - 1], radius=r, outline=rgb(col))

    def fill_circle(self, x, y, r, col):
        self.d.ellipse([x - r, y - r, x + r, y + r], fill=rgb(col))

    def draw_circle(self, x, y, r, col):
        self.d.ellipse([x - r, y - r, x + r, y + r], outline=rgb(col))

    def draw_line(self, x0, y0, x1, y1, col):
        self.d.line([x0, y0, x1, y1], fill=rgb(col))

    def fill_triangle(self, x0, y0, x1, y1, x2, y2, col):
        self.d.polygon([(x0, y0), (x1, y1), (x2, y2)], fill=rgb(col))

    # text -----------------------------------------------------------------
    def set_text_size(self, s):
        self.tsize = s

    def set_text_color(self, col):
        self.tcolor = col

    def set_cursor(self, x, y):
        self.cx, self.cy = x, y

    def _glyph(self, ch, x, y):
        s = self.tsize
        cols = FONT[ord(ch) & 0xFF]
        col = rgb(self.tcolor)
        for cxp in range(5):
            bits = cols[cxp]
            for ryp in range(8):
                if bits & (1 << ryp):
                    px = x + cxp * s
                    py = y + ryp * s
                    self.d.rectangle([px, py, px + s - 1, py + s - 1], fill=col)

    def print(self, text):
        for ch in text:
            self._glyph(ch, self.cx, self.cy)
            self.cx += 6 * self.tsize

    def save(self, name):
        os.makedirs(OUT_DIR, exist_ok=True)
        big = self.img.resize((DISPLAY_W * SCALE, DISPLAY_H * SCALE), Image.NEAREST)
        big.save(os.path.join(OUT_DIR, name))
        print("wrote", name)


# --- Wrapped text (ports drawWrappedText) ----------------------------------

def draw_wrapped_text(s, text, x, y, maxWpx, col, lineH):
    s.set_text_size(1)
    s.set_text_color(col)
    maxChars = maxWpx // 6
    words = text.split(" ")
    line = ""
    cy = y
    for w in words:
        need = (len(line) + 1 if line else 0) + len(w)
        if need > maxChars and line:
            s.set_cursor(x, cy)
            s.print(line)
            cy += lineH
            line = ""
        if line:
            line += " "
        line += w
    if line:
        s.set_cursor(x, cy)
        s.print(line)

# --- Speech bubble (ports drawSpeechBubble) ---------------------------------

def draw_speech_bubble(s, x, y, w, h, col):
    s.draw_round_rect(x, y, w, h, 6, col)
    ty = y + 20
    s.draw_line(x, ty - 5, x, ty + 5, THEME["background"])
    s.draw_line(x, ty - 5, x - 8, ty, col)
    s.draw_line(x, ty + 5, x - 8, ty, col)

# --- Dwight ASCII portrait (ports DwightFace.cpp) ---------------------------

def load_ascii():
    txt = open(ASCII_H).read()
    m = re.search(r'R"DWIGHT\((.*?)\)DWIGHT"', txt, re.S)
    art = m.group(1)
    if art.startswith("\n"):
        art = art[1:]
    return art.split("\n")

ASCII_LINES = [l for l in load_ascii()]
# drop a trailing empty line from the raw literal if present
while ASCII_LINES and ASCII_LINES[-1] == "":
    ASCII_LINES.pop()

def char_level(ch):
    if ch in " \t":
        return 0
    if ch in ".`',-:_\";":
        return 1
    if ch in "/+\\<>!|*=oscrvzxilt":
        return 2
    if ch in "yhdmnuwqpkea":
        return 3
    if ch in "NMWHDQRB@#&%OG":
        return 4
    return 3

def face_width():
    return max((len(l) for l in ASCII_LINES), default=0)

def face_height():
    return len(ASCII_LINES) * 2

def draw_dwight(s, leftX):
    lc = [0] * 5
    lc[1] = lerp565(THEME["background"], THEME["muted"], 150)
    lc[2] = THEME["muted"]
    lc[3] = lerp565(THEME["muted"], THEME["foreground"], 140)
    lc[4] = THEME["foreground"]
    rows = len(ASCII_LINES)
    cellW, cellH = 1, 2
    y0 = (DISPLAY_H - rows * cellH) // 2
    for row, line in enumerate(ASCII_LINES):
        for col, ch in enumerate(line):
            lvl = char_level(ch)
            if lvl > 0:
                s.fill_rect(leftX + col * cellW, y0 + row * cellH, cellW, cellH, lc[lvl])

# ===========================================================================
# Screens
# ===========================================================================

def screen_menu():
    s = Screen()
    items = ["Slide Sensei", "Bears Don't Sleep", "Ghost Writer", "Open Sesame",
             "FALSE.", "Back To Work", "Schrute Wisdom", "Beet Farm WiFi", "Config"]
    selection = 0
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    draw_dwight(s, 0)

    visible = 4
    hasScrollbar = len(items) > visible
    sbW, sbGap = 3, 3
    rightReserve = (sbW + sbGap) if hasScrollbar else 0
    listX = face_width() + 6
    listW = DISPLAY_W - listX - 2 - rightReserve
    maxChars = listW // 6

    rowH = 30
    top = selection - visible // 2
    if top < 0:
        top = 0
    if top + visible > len(items):
        top = max(0, len(items) - visible)
    shown = min(visible, len(items) - top)
    y = max(4, (DISPLAY_H - shown * rowH) // 2)
    for i in range(shown):
        idx = top + i
        sel = idx == selection
        name = items[idx]
        # wrap to <=2 lines
        words = name.split(" ")
        lines, cur = [], ""
        for w in words:
            if not cur:
                cur = w
            elif len(cur) + 1 + len(w) <= maxChars:
                cur += " " + w
            else:
                lines.append(cur)
                cur = w
            if len(lines) == 2:
                break
        if cur and len(lines) < 2:
            lines.append(cur)
        if sel:
            s.fill_round_rect(listX - 4, y, listW + 4, rowH - 4, 3, THEME["accent"])
            s.set_text_color(THEME["background"])
        else:
            s.set_text_color(THEME["foreground"])
        ty = y + (rowH - 4 - len(lines) * 11) // 2 + 1
        for ln in lines:
            s.set_cursor(listX, ty)
            s.print(ln)
            ty += 11
        y += rowH

    if hasScrollbar:
        total = len(items)
        sbX = DISPLAY_W - sbW - 1
        sbY, sbH = 4, DISPLAY_H - 8
        s.fill_round_rect(sbX, sbY, sbW, sbH, 1, THEME["muted"])
        thumbH = max(8, sbH * visible // total)
        maxTop = total - visible
        thumbY = sbY + (sbH - thumbH) * top // maxTop
        s.fill_round_rect(sbX, thumbY, sbW, thumbH, 1, THEME["foreground"])
    s.save("01-menu.png")


def _status(s, text, color, with_icon=False, connected=False):
    if with_icon:
        ICON_W, ICON_PAD = 7, 2
        totalW = ICON_W + ICON_PAD + 6 * len(text)
        startX = DISPLAY_W - totalW
        icx, icy = startX + ICON_W // 2, 4
        if connected:
            s.fill_circle(icx, icy, 3, color)
        else:
            s.draw_circle(icx, icy, 3, color)
            s.draw_line(icx, icy, icx, icy - 2, color)
            s.draw_line(icx, icy, icx + 2, icy, color)
        s.set_text_color(color)
        s.set_cursor(startX + ICON_W + ICON_PAD, 1)
        s.print(text)
    else:
        sw = 6 * len(text)
        s.set_text_color(color)
        s.set_cursor(DISPLAY_W - sw - 2, 1)
        s.print(text)


def screen_presentation():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    _status(s, "CONNECTED", THEME["success"], with_icon=True, connected=True)
    draw_dwight(s, 0)
    colX = face_width() + 6
    colW = DISPLAY_W - colX - 4
    tbuf = "00:42"
    s.set_text_size(3)
    s.set_text_color(THEME["foreground"])
    tw = 18 * len(tbuf)
    s.set_cursor(colX + (colW - tw) // 2, 16)
    s.print(tbuf)
    bx, by = colX, 48
    bw, bh = colW, DISPLAY_H - by - 5
    draw_speech_bubble(s, bx, by, bw, bh, THEME["accent"])
    draw_wrapped_text(s, "Ready. Press , . ; / and I move your slides.",
                      bx + 8, by + 10, bw - 16, THEME["foreground"], 11)
    s.save("02-slide-sensei.png")


def screen_amphetamine():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    _status(s, "AWAKE", THEME["success"])
    draw_dwight(s, 0)
    faceTop = (DISPLAY_H - face_height()) // 2
    # eyes "on" frame
    s.fill_circle(38, faceTop + 62, 3, RED_EYE)
    s.fill_circle(66, faceTop + 62, 3, RED_EYE)
    bx, by = face_width() + 6, 22
    bw, bh = DISPLAY_W - bx - 4, 100
    draw_speech_bubble(s, bx, by, bw, bh, THEME["accent"])
    draw_wrapped_text(s, "Wired and wide awake. No naps on my watch.",
                      bx + 8, by + 10, bw - 16, THEME["foreground"], 11)
    s.save("03-bears-dont-sleep.png")


def screen_keyboard():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    _status(s, "ONLINE", THEME["success"])
    draw_dwight(s, 0)
    bx, by = face_width() + 6, 22
    bw, bh = DISPLAY_W - bx - 4, 100
    draw_speech_bubble(s, bx, by, bw, bh, THEME["accent"])
    draw_wrapped_text(s, "I record everything you press.",
                      bx + 8, by + 12, bw - 16, THEME["foreground"], 11)
    s.set_text_color(THEME["muted"])
    s.set_cursor(bx + 8, by + bh - 14)
    s.print("ESC = exit")
    s.save("04-ghost-writer.png")


def screen_unlock():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    _status(s, "CONNECTED", THEME["success"])
    draw_dwight(s, 0)
    colX = face_width() + 6
    colW = DISPLAY_W - colX - 4
    s.set_text_color(THEME["accent"])
    s.set_cursor(colX, 14)
    s.print("UNLOCK")
    computers = ["Work laptop", "Home tower", "Lab rig"]
    selection = 0
    listTop, rowH = 30, 16
    maxChars = colW // 6 - 1
    y = listTop
    for idx, nm in enumerate(computers):
        sel = idx == selection
        if sel:
            s.fill_round_rect(colX - 2, y - 1, colW + 2, rowH - 2, 2, THEME["accent"])
            s.set_text_color(THEME["background"])
        else:
            s.set_text_color(THEME["foreground"])
        s.set_cursor(colX, y + 3)
        s.print(nm[:maxChars])
        y += rowH
    s.set_text_color(THEME["muted"])
    s.set_cursor(colX, DISPLAY_H - 12)
    s.print("Enter = unlock")
    s.save("05-open-sesame.png")


def screen_answers():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    x = 6
    w = DISPLAY_W - 12
    maxChars = w // 6
    s.set_text_color(THEME["accent"])
    s.set_cursor(x, 3)
    s.print("DWIGHT ANSWERS")
    # BLE status, top-right (this app draws its status row at y=3, not y=1).
    s.set_text_color(THEME["success"])
    s.set_cursor(DISPLAY_W - 6 * len("ONLINE") - 2, 3)
    s.print("ONLINE")
    phrases = ["Sounds good, I'll take care of it.", "Let me circle back on that.",
               "Per my last email...", "Adding this to the agenda.",
               "Identity theft is not a joke."]
    selection = 0
    listTop, rowH = 22, 18
    maxRows = (DISPLAY_H - listTop - 14) // rowH
    top = 0
    shown = min(maxRows, len(phrases) - top)
    y = listTop
    for i in range(shown):
        idx = top + i
        sel = idx == selection
        if sel:
            s.fill_round_rect(x - 2, y - 1, w + 2, rowH - 2, 2, THEME["accent"])
            s.set_text_color(THEME["background"])
        else:
            s.set_text_color(THEME["foreground"])
        ph = phrases[idx]
        if len(ph) > maxChars:
            ph = ph[:maxChars - 1] + "~"
        s.set_cursor(x, y + 4)
        s.print(ph)
        y += rowH
    s.set_text_color(THEME["muted"])
    s.set_cursor(x, DISPLAY_H - 11)
    s.print("Ent=type n=new e=edit d=del")
    s.save("06-false.png")


def screen_focus():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    duration = 1500 * 1000
    remaining = 1104 * 1000  # 18:24
    s.set_text_color(THEME["accent"])
    s.set_cursor(2, 2)
    s.print("FOCUS")
    barX, barY, barW, barH = 2, 14, DISPLAY_W - 4, 5
    s.draw_rect(barX, barY, barW, barH, THEME["muted"])
    elapsed = duration - remaining
    fill = (barW - 2) * elapsed // duration
    if fill > 0:
        s.fill_rect(barX + 1, barY + 1, fill, barH - 2, THEME["accent"])
    # big time size 5
    buf = "18:24"
    size = 5
    charW = 6 * size
    w = len(buf) * charW
    s.set_text_size(size)
    s.set_text_color(THEME["foreground"])
    s.set_cursor((DISPLAY_W - w) // 2, 46)
    s.print(buf)
    s.set_text_size(1)
    by = 96
    bh = DISPLAY_H - by - 2
    draw_speech_bubble(s, 2, by, DISPLAY_W - 4, bh, THEME["warning"])
    draw_wrapped_text(s, "Bears. Beets. Deadlines. FOCUS, drone.",
                      10, by + 8, DISPLAY_W - 20, THEME["foreground"], 10)
    s.save("07-back-to-work.png")


def screen_coaching():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    draw_dwight(s, 0)
    bx, by = face_width() + 6, 8
    bw, bh = DISPLAY_W - bx - 4, DISPLAY_H - by - 16
    draw_speech_bubble(s, bx, by, bw, bh, THEME["accent"])
    quote = ("Discipline equals freedom. Beets equal power. "
             "You equal mine to command.")
    draw_wrapped_text(s, quote, bx + 8, by + 8, bw - 16, THEME["foreground"], 11)
    s.set_text_color(THEME["muted"])
    s.set_cursor(2, DISPLAY_H - 11)
    s.print("Any key = next  `=back")
    s.save("08-schrute-wisdom.png")


def screen_beetfarm():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    statusStr = "AP ON"
    sw = 6 * len(statusStr)
    s.fill_circle(DISPLAY_W - sw - 8, 4, 3, THEME["success"])
    s.set_text_color(THEME["success"])
    s.set_cursor(DISPLAY_W - sw - 2, 1)
    s.print(statusStr)
    draw_dwight(s, 0)
    colX = face_width() + 6
    colW = DISPLAY_W - colX - 4
    bx, by = colX, 16
    bw, bh = colW, DISPLAY_H - by - 5
    draw_speech_bubble(s, bx, by, bw, bh, THEME["accent"])
    ix, iw = bx + 8, bw - 16
    y = by + 9
    s.set_text_color(THEME["accent"]); s.set_cursor(ix, y); s.print("BEET TRAP LIVE"); y += 14
    s.set_text_color(THEME["muted"]); s.set_cursor(ix, y); s.print("SSID"); y += 10
    draw_wrapped_text(s, "Schrute_Farms_Free_WiFi", ix, y, iw, THEME["foreground"], 10); y += 21
    s.set_text_color(THEME["foreground"]); s.set_cursor(ix, y); s.print("IP 192.168.4.1"); y += 12
    s.set_text_color(THEME["success"]); s.set_cursor(ix, y); s.print("Victims: 2")
    s.set_text_color(THEME["muted"]); s.set_cursor(ix, by + bh - 13); s.print("ESC = exit")
    s.save("09-beet-farm-wifi.png")


def screen_config():
    s = Screen()
    s.fill_screen(THEME["background"])
    s.set_text_size(1)
    x = 6
    w = DISPLAY_W - 12
    s.set_text_color(THEME["accent"])
    s.set_cursor(x, 3)
    s.print("CONFIG")
    sections = ["Computers", "Appearance", "Sound", "Power"]
    sectionSel = 0
    listTop, rowH = 24, 20
    y = listTop
    for i, name in enumerate(sections):
        sel = i == sectionSel
        if sel:
            s.fill_round_rect(x - 2, y - 1, w + 2, rowH - 2, 2, THEME["accent"])
            s.set_text_color(THEME["background"])
        else:
            s.set_text_color(THEME["foreground"])
        s.set_cursor(x, y + 4)
        s.print(name)
        y += rowH
    s.set_text_color(THEME["muted"])
    s.set_cursor(x, DISPLAY_H - 11)
    s.print("Enter=open  ;/.=move  `=back")
    s.save("10-config.png")


if __name__ == "__main__":
    screen_menu()
    screen_presentation()
    screen_amphetamine()
    screen_keyboard()
    screen_unlock()
    screen_answers()
    screen_focus()
    screen_coaching()
    screen_beetfarm()
    screen_config()
    print("done ->", OUT_DIR)
