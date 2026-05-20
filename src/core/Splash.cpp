#include "Splash.h"

#include <stdio.h>

#include "DwightFace.h"
#include "HardwareConfig.h"
#include "Theme.h"
#include "Version.h"

void splashShow(M5Canvas* canvas, uint32_t duration_ms) {
    const uint32_t frameMs = 400;
    const int tx = 100;  // start of the right-hand text column
    uint32_t elapsed = 0;
    int dots = 0;
    while (true) {
        canvas->fillScreen(gTheme.background);
        dwightDrawAsciiFace(canvas, 0);  // portrait flush-left

        canvas->setTextColor(gTheme.foreground);
        canvas->setTextSize(2);
        canvas->setCursor(tx, 16);
        canvas->print("DWIGHT");
        canvas->setCursor(tx, 38);
        canvas->print("ASSISTANT");

        canvas->setTextSize(1);
        canvas->setTextColor(gTheme.muted);
        canvas->setCursor(tx, 62);
        canvas->print("v" FIRMWARE_VERSION);

        canvas->setTextColor(gTheme.foreground);
        canvas->setCursor(tx, 92);
        canvas->print("Dwight will assist");
        char buf[12];
        snprintf(buf, sizeof(buf), "you%.*s", dots, "...");
        canvas->setCursor(tx, 104);
        canvas->print(buf);

        canvas->pushSprite(0, 0);

        if (elapsed >= duration_ms) break;
        delay(frameMs);
        elapsed += frameMs;
        dots = (dots + 1) % 4;
    }
}
