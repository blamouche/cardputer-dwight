#include "DwightFace.h"

#include "HardwareConfig.h"
#include "Theme.h"
#include "dwight_ascii.h"

namespace {

// Maps an ASCII glyph to a brightness level 0..4 by visual density.
int charLevel(char c) {
    switch (c) {
        case ' ': case '\t':
            return 0;
        case '.': case '`': case '\'': case ',':
        case '-': case ':': case '_': case '"': case ';':
            return 1;
        case '/': case '+': case '\\': case '<': case '>': case '!': case '|': case '*': case '=':
        case 'o': case 's': case 'c': case 'r': case 'v': case 'z': case 'x': case 'i': case 'l': case 't':
            return 2;
        case 'y': case 'h': case 'd': case 'm': case 'n': case 'u': case 'w': case 'q': case 'p': case 'k': case 'e': case 'a':
            return 3;
        case 'N': case 'M': case 'W': case 'H': case 'D': case 'Q': case 'R': case 'B':
        case '@': case '#': case '&': case '%': case 'O': case 'G':
            return 4;
        default:
            return 3;
    }
}

const char* skipLeadingNewline(const char* p) {
    return (*p == '\n') ? p + 1 : p;
}

// Linear blend between two RGB565 colors. t = 0 returns a, t = 255 returns b.
uint16_t lerp565(uint16_t a, uint16_t b, uint8_t t) {
    int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
    int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
    int r = ar + (br - ar) * t / 255;
    int g = ag + (bg - ag) * t / 255;
    int bl = ab + (bb - ab) * t / 255;
    return (uint16_t)((r << 11) | (g << 5) | bl);
}

}  // namespace

int dwightAsciiFaceWidth() {
    const char* p = skipLeadingNewline(DWIGHT_ASCII);
    int maxW = 0, w = 0;
    for (const char* q = p; *q; q++) {
        if (*q == '\n') { if (w > maxW) maxW = w; w = 0; }
        else w++;
    }
    if (w > maxW) maxW = w;
    return maxW;  // cellW == 1
}

int dwightAsciiFaceHeight() {
    const char* p = skipLeadingNewline(DWIGHT_ASCII);
    int rows = 0, w = 0;
    for (const char* q = p; *q; q++) {
        if (*q == '\n') { w = 0; rows++; } else { w++; }
    }
    if (w > 0) rows++;
    return rows * 2;  // cellH == 2
}

void dwightDrawAsciiFace(M5Canvas* canvas, int leftX) {
    // Build the 5-level luminance ramp entirely from the active theme so the
    // portrait follows any palette (including the white/grayscale "light"
    // theme) instead of leaking hardcoded green at the mid tones.
    uint16_t levelColor[5];
    levelColor[0] = gTheme.background;                              // unused (skipped)
    levelColor[1] = lerp565(gTheme.background, gTheme.muted, 150);  // faint
    levelColor[2] = gTheme.muted;                                   // dim
    levelColor[3] = lerp565(gTheme.muted, gTheme.foreground, 140);  // mid
    levelColor[4] = gTheme.foreground;                              // bright

    const char* p = skipLeadingNewline(DWIGHT_ASCII);

    int rows = 0, w = 0;
    for (const char* q = p; *q; q++) {
        if (*q == '\n') { w = 0; rows++; } else { w++; }
    }
    if (w > 0) rows++;

    // Monospace glyph cells are ~twice as tall as wide; reproduce that aspect
    // so the portrait isn't squashed (cellH = 2 * cellW).
    const int cellW = 1;
    const int cellH = 2;
    const int y0 = (DISPLAY_H - rows * cellH) / 2;  // may be negative; clips

    int row = 0, col = 0;
    for (const char* q = p; *q; q++) {
        if (*q == '\n') { row++; col = 0; continue; }
        int lvl = charLevel(*q);
        if (lvl > 0) {
            canvas->fillRect(leftX + col * cellW, y0 + row * cellH,
                             cellW, cellH, levelColor[lvl]);
        }
        col++;
    }
}
