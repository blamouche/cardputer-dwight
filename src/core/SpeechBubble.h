#pragma once

#include <M5Cardputer.h>

// Outline speech bubble with a pointed tail on its left edge (pointing toward
// the portrait drawn to its left).
void drawSpeechBubble(M5Canvas* canvas, int x, int y, int w, int h, uint16_t color);

// Word-wrap `text` into a box `maxWpx` pixels wide at the size-1 font, starting
// at (x, y) and advancing `lineH` pixels per line.
void drawWrappedText(M5Canvas* canvas, const char* text, int x, int y,
                     int maxWpx, uint16_t color, int lineH);
