#include "Theme.h"
#include <M5Cardputer.h>
#include <math.h>
#include <string.h>

Theme gTheme;

namespace {

// A theme preset stored as raw 8-bit RGB triplets so the table stays readable;
// the values are converted to 565 when applied (color565 needs the display).
struct ThemePreset {
    const char* id;
    const char* label;
    uint8_t bg[3];
    uint8_t fg[3];
    uint8_t accent[3];
    uint8_t muted[3];
    uint8_t success[3];
    uint8_t warning[3];
    uint8_t error[3];
};

// Dark "phosphor" themes share a black background and an amber/red alert pair;
// only the text/selection/dim tints change. The "light" theme inverts to a
// white background rendered fully in grayscale.
const ThemePreset kPresets[] = {
    // Soft terminal green — the original look and the default.
    {"green",  "Green",  {0x00,0x00,0x00}, {0x33,0xCC,0x44}, {0x40,0xD8,0x5C},
     {0x0E,0x55,0x24}, {0x33,0xCC,0x44}, {0xE0,0xB0,0x00}, {0xCC,0x33,0x33}},
    // Cool blue.
    {"blue",   "Blue",   {0x00,0x00,0x00}, {0x33,0x99,0xEE}, {0x55,0xB0,0xFF},
     {0x12,0x3A,0x66}, {0x33,0x99,0xEE}, {0xE0,0xB0,0x00}, {0xCC,0x33,0x33}},
    // Violet / purple.
    {"purple", "Purple", {0x00,0x00,0x00}, {0xB0,0x66,0xEE}, {0xC9,0x88,0xFF},
     {0x3A,0x1E,0x5C}, {0xB0,0x66,0xEE}, {0xE0,0xB0,0x00}, {0xCC,0x33,0x33}},
    // Warm yellow.
    {"yellow", "Yellow", {0x00,0x00,0x00}, {0xE6,0xC8,0x2E}, {0xFF,0xDD,0x44},
     {0x5C,0x4E,0x10}, {0xE6,0xC8,0x2E}, {0xE0,0xB0,0x00}, {0xCC,0x33,0x33}},
    // White background, full grayscale interface.
    {"light",  "Light",  {0xFF,0xFF,0xFF}, {0x22,0x22,0x22}, {0x44,0x44,0x44},
     {0x88,0x88,0x88}, {0x33,0x33,0x33}, {0x55,0x55,0x55}, {0x11,0x11,0x11}},
    // Animated rainbow "pride": foreground/accent/muted cycle through the hue
    // wheel every frame (see themeTick). The values below are only the seed
    // shown before the first tick; background and status colors stay put.
    {"pride",  "Pride",  {0x00,0x00,0x00}, {0xFF,0x3D,0xA6}, {0xFF,0x7A,0x18},
     {0x5A,0x2A,0x8C}, {0x33,0xCC,0x44}, {0xF2,0xC8,0x18}, {0xE0,0x33,0x33}},
};

const int kPresetCount = sizeof(kPresets) / sizeof(kPresets[0]);

// Index of the live theme, so themeTick() knows whether to animate.
int gActiveIdx = 0;

// Full hue-wheel sweep period for the animated "pride" theme, in ms.
const uint32_t kPridePeriodMs = 6000;

uint16_t to565(const uint8_t rgb[3]) {
    return M5Cardputer.Display.color565(rgb[0], rgb[1], rgb[2]);
}

// HSV (h in [0,360), s/v in [0,1]) -> RGB565. Used by the animated theme.
uint16_t hsv565(float h, float s, float v) {
    h = fmodf(h, 360.0f);
    if (h < 0) h += 360.0f;
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0, g = 0, b = 0;
    if (h < 60)       { r = c; g = x; }
    else if (h < 120) { r = x; g = c; }
    else if (h < 180) { g = c; b = x; }
    else if (h < 240) { g = x; b = c; }
    else if (h < 300) { r = x; b = c; }
    else              { r = c; b = x; }
    return M5Cardputer.Display.color565((uint8_t)((r + m) * 255),
                                        (uint8_t)((g + m) * 255),
                                        (uint8_t)((b + m) * 255));
}

bool isPrideIdx(int idx) {
    return strcmp(kPresets[idx].id, "pride") == 0;
}

void applyPreset(const ThemePreset& p) {
    gTheme.background = to565(p.bg);
    gTheme.foreground = to565(p.fg);
    gTheme.accent     = to565(p.accent);
    gTheme.muted      = to565(p.muted);
    gTheme.success    = to565(p.success);
    gTheme.warning    = to565(p.warning);
    gTheme.error      = to565(p.error);
    gTheme.brightness = 255;
}

}  // namespace

void themeApply(const String& id) {
    int idx = themeIndexOf(id);
    if (idx < 0) idx = 0;  // default to "green"
    gActiveIdx = idx;
    applyPreset(kPresets[idx]);
}

void themeApplyDefaults() {
    gActiveIdx = 0;
    applyPreset(kPresets[0]);
}

void themeTick(uint32_t nowMs) {
    if (!isPrideIdx(gActiveIdx)) return;  // only the animated theme reacts

    // Sweep the full hue wheel once per period. The background stays black and
    // the status colors keep the preset values (set by applyPreset) so warnings
    // and errors remain legible; only the three "chrome" roles cycle.
    float hue = (float)(nowMs % kPridePeriodMs) * 360.0f / (float)kPridePeriodMs;
    gTheme.foreground = hsv565(hue,        0.85f, 1.00f);
    gTheme.accent     = hsv565(hue + 40.0f, 0.90f, 1.00f);
    gTheme.muted      = hsv565(hue + 20.0f, 0.70f, 0.35f);  // dim track/labels
}

int themeCount() { return kPresetCount; }

const char* themeId(int index) {
    if (index < 0 || index >= kPresetCount) return kPresets[0].id;
    return kPresets[index].id;
}

const char* themeLabel(int index) {
    if (index < 0 || index >= kPresetCount) return kPresets[0].label;
    return kPresets[index].label;
}

int themeIndexOf(const String& id) {
    for (int i = 0; i < kPresetCount; ++i) {
        if (id == kPresets[i].id) return i;
    }
    return -1;
}
