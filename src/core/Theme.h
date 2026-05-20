#pragma once

#include <Arduino.h>
#include <stdint.h>

struct Theme {
    uint16_t background;
    uint16_t foreground;
    uint16_t accent;
    uint16_t muted;
    uint16_t success;
    uint16_t warning;
    uint16_t error;
    uint8_t  brightness;
};

extern Theme gTheme;

// Applies the default ("green") palette. Kept for callers that just want a
// sane palette before the SD config has been read.
void themeApplyDefaults();

// Applies a named color preset to gTheme. Unknown ids fall back to "green".
// Available ids: "green", "blue", "purple", "yellow", "light", "pride".
void themeApply(const String& id);

// Advances time-based theme animation. Call once per frame from the main loop
// with millis(). Only the animated "pride" theme reacts (it cycles its
// foreground/accent/muted through the hue wheel); for every other theme this is
// a no-op, so static themes are unaffected.
void themeTick(uint32_t nowMs);

// Catalog of selectable themes, used by the on-device config picker.
int         themeCount();
const char* themeId(int index);     // stable id stored in the config
const char* themeLabel(int index);  // human-readable label for the UI
int         themeIndexOf(const String& id);  // -1 if unknown
