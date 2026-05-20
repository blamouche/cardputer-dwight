#include "SpeechBubble.h"

#include "Theme.h"

void drawSpeechBubble(M5Canvas* canvas, int x, int y, int w, int h, uint16_t color) {
    canvas->drawRoundRect(x, y, w, h, 6, color);
    int ty = y + 20;
    canvas->drawLine(x, ty - 5, x, ty + 5, gTheme.background);  // open the mouth
    canvas->drawLine(x, ty - 5, x - 8, ty, color);
    canvas->drawLine(x, ty + 5, x - 8, ty, color);
}

void drawWrappedText(M5Canvas* canvas, const char* text, int x, int y,
                     int maxWpx, uint16_t color, int lineH) {
    canvas->setTextSize(1);
    canvas->setTextColor(color);
    const int maxChars = maxWpx / 6;
    char line[48];
    int len = 0, cy = y;
    const char* p = text;
    while (*p) {
        const char* ws = p;
        while (*p && *p != ' ') p++;
        int wlen = (int)(p - ws);
        int need = (len ? len + 1 : 0) + wlen;
        if (need > maxChars && len > 0) {
            line[len] = 0;
            canvas->setCursor(x, cy);
            canvas->print(line);
            cy += lineH;
            len = 0;
        }
        if (len > 0 && len < (int)sizeof(line) - 1) line[len++] = ' ';
        for (int i = 0; i < wlen && len < (int)sizeof(line) - 1; i++) line[len++] = ws[i];
        while (*p == ' ') p++;
    }
    if (len > 0) {
        line[len] = 0;
        canvas->setCursor(x, cy);
        canvas->print(line);
    }
}
