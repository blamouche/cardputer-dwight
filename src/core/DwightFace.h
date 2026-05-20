#pragma once

#include <M5Cardputer.h>

// Renders the full ASCII-art Dwight portrait (the same one shown on the boot
// splash) as a luminance bitmap. The portrait's left edge is placed at
// `leftX` and it is vertically centered on the display. The 5-level brightness
// ramp is derived from gTheme (background -> muted -> foreground), so the
// portrait follows the active color theme.
void dwightDrawAsciiFace(M5Canvas* canvas, int leftX);

// Rendered width in pixels (widest art line, 1px per glyph). Useful for laying
// out content to the right of the portrait.
int dwightAsciiFaceWidth();

// Rendered height in pixels (row count * 2). The portrait is vertically
// centered, so its top edge sits at (DISPLAY_H - height) / 2.
int dwightAsciiFaceHeight();
