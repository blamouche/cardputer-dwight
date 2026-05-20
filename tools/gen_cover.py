#!/usr/bin/env python3
"""Compose the M5Burner cover image for Dwight Assistant.

Reuses the firmware-faithful renderer from gen_screenshots.py: the real glcdfont,
the Dwight ASCII portrait and the phosphor-green theme, plus the already-generated
app screenshots as a bottom filmstrip.

Run:  tools/gen_cover.py        (needs Pillow; same venv as gen_screenshots.py)
Out:  docs/m5burner-cover.png
"""
import os
from PIL import Image, ImageDraw

import gen_screenshots as gs

OUT = os.path.join(gs.ROOT, "docs", "m5burner-cover.png")
W, H = 1000, 600


def draw_text(d, text, x, y, size, color565):
    """glcdfont text at an arbitrary scale (mirrors Screen.print)."""
    col = gs.rgb(color565)
    cx = x
    for ch in text:
        cols = gs.FONT[ord(ch) & 0xFF]
        for cxp in range(5):
            bits = cols[cxp]
            for ryp in range(8):
                if bits & (1 << ryp):
                    px = cx + cxp * size
                    py = y + ryp * size
                    d.rectangle([px, py, px + size - 1, py + size - 1], fill=col)
        cx += 6 * size
    return cx  # x advance end


def text_w(text, size):
    return len(text) * 6 * size


def draw_portrait(d, left, top, scale):
    """Dwight ASCII portrait at `scale` (cellW=scale, cellH=2*scale)."""
    lc = [0] * 5
    lc[1] = gs.lerp565(gs.THEME["background"], gs.THEME["muted"], 150)
    lc[2] = gs.THEME["muted"]
    lc[3] = gs.lerp565(gs.THEME["muted"], gs.THEME["foreground"], 140)
    lc[4] = gs.THEME["foreground"]
    cw, ch = scale, scale * 2
    for row, line in enumerate(gs.ASCII_LINES):
        for col, c in enumerate(line):
            lvl = gs.char_level(c)
            if lvl > 0:
                px = left + col * cw
                py = top + row * ch
                d.rectangle([px, py, px + cw - 1, py + ch - 1], fill=gs.rgb(lc[lvl]))


def main():
    bg = gs.rgb(gs.THEME["background"])
    fg = gs.THEME["foreground"]
    accent = gs.THEME["accent"]
    muted = gs.THEME["muted"]

    img = Image.new("RGB", (W, H), bg)
    d = ImageDraw.Draw(img)

    # Thin phosphor frame.
    d.rounded_rectangle([6, 6, W - 7, H - 7], radius=14, outline=gs.rgb(muted), width=2)

    # --- Portrait, upper-left ------------------------------------------------
    pscale = 3
    pw = gs.face_width() * pscale
    ph = len(gs.ASCII_LINES) * pscale * 2
    px0, py0 = 40, 26
    draw_portrait(d, px0, py0, pscale)

    # --- Title block, upper-right -------------------------------------------
    tx = px0 + pw + 48
    draw_text(d, "DWIGHT", tx, 60, 9, fg)
    draw_text(d, "ASSISTANT", tx, 150, 6, accent)

    draw_text(d, "M5Stack Cardputer office firmware", tx, 232, 2, fg)
    draw_text(d, "9 apps  BLE  WiFi  SD-config", tx, 262, 2, muted)

    # Version pill.
    ver = "v" + open(os.path.join(gs.ROOT, ".prompt-hub/version.md")).read().strip()
    vw = text_w(ver, 3)
    d.rounded_rectangle([tx, 300, tx + vw + 24, 300 + 36], radius=8, fill=gs.rgb(accent))
    draw_text(d, ver, tx + 12, 307, 3, gs.THEME["background"])

    # --- Bottom filmstrip ----------------------------------------------------
    strip = ["01-menu.png", "02-slide-sensei.png", "07-back-to-work.png",
             "09-beet-farm-wifi.png"]
    n = len(strip)
    margin = 40
    gap = 18
    tw = (W - 2 * margin - (n - 1) * gap) // n          # thumb width
    th = round(tw * gs.DISPLAY_H / gs.DISPLAY_W)         # keep 240:135 ratio
    sy = H - margin - th
    x = margin
    for name in strip:
        thumb = Image.open(os.path.join(gs.OUT_DIR, name)).resize((tw, th), Image.NEAREST)
        img.paste(thumb, (x, sy))
        d.rectangle([x, sy, x + tw - 1, sy + th - 1], outline=gs.rgb(muted))
        x += tw + gap

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    img.save(OUT)
    print("wrote", OUT, f"({W}x{H})  portrait {pw}x{ph}")


if __name__ == "__main__":
    main()
